#[cfg(test)]
mod test;

use crate::Value::{self, Null};

// State machine parser, only states that may traverse whitespace are allowed.
// Strings, numbers, and primitives are parsed in a single step.
#[allow(non_camel_case_types)]
#[derive(Debug, PartialEq)]
enum State {
    // Initial state, expecting the first value
    VALUE,

    // Start of an array, expecting a value or end
    ARRAY_START,

    // Parsing an array, expecting a value
    ARRAY_VALUE,

    // Parsing an array, expecting a ',' or ']'
    ARRAY_END,

    // Start of an object, expecting a key or end
    OBJECT_START,

    // Expecting a string key for an object
    OBJECT_KEY,

    // Expecting a ':' between a string key and a value
    OBJECT_COLON,

    // Parsing an object, expecting a value
    OBJECT_VALUE,

    // Parsing an object, expecting a ',' or '}'
    OBJECT_END,

    // First value parsed, expecting end of input
    END,
}

// Simple type enum to track nested structures
#[derive(Debug)]
enum StructureType {
    ARRAY,
    OBJECT,
}

pub fn parse(input: &str) -> Result<Value, Error> {
    validate(input)?;

    optimistic(input).ok_or(Error::unknown())
}

// Using macros just to avoid any chance of functional overhead or custom structs
// A lot of code is shared between validate and optimistic

macro_rules! format_byte {
    ($b:expr) => {
        match $b {
            // Things that don't go well in quotes
            b'\'' | b'"' | b'`' => format!("token ({})", $b as char),
            b' '..=b'~' => format!("token '{}'", $b as char),
            _ => format!("byte ({})", $b),
        }
    };
}

macro_rules! is_byte_order_mark {
    ($bytes:expr) => {
        $bytes[0] == 0xEF && $bytes[1] == 0xBB && $bytes[2] == 0xBF
    };
}

macro_rules! is_whitespace {
    ($b:expr) => {
        $b == b' ' || $b == b'\n' || $b == b'\r' || $b == b'\t'
    };
}

macro_rules! pop_stack {
    ($stack:expr, $state:expr) => {
        $stack.pop();
        match $stack.last() {
            None => $state = END,
            Some(ARRAY) => $state = ARRAY_END,
            Some(OBJECT) => $state = OBJECT_END,
        }
    };
}

macro_rules! next_value {
    ($state:expr, $err:expr) => {
        $state = match $state {
            ARRAY_START | ARRAY_VALUE => ARRAY_END,
            OBJECT_VALUE => OBJECT_END,
            VALUE => END,
            _ => {
                return $err;
            }
        }
    };
}

macro_rules! is_null {
    ($bytes:expr, $size:expr, $index:expr) => {
        $bytes[$index] == b'n'
            && $index + 3 < $size
            && $bytes[$index + 1] == b'u'
            && $bytes[$index + 2] == b'l'
            && $bytes[$index + 3] == b'l'
    };
}

macro_rules! is_true {
    ($bytes:expr, $size:expr, $index:expr) => {
        $bytes[$index] == b't'
            && $index + 3 < $size
            && $bytes[$index + 1] == b'r'
            && $bytes[$index + 2] == b'u'
            && $bytes[$index + 3] == b'e'
    };
}

macro_rules! is_false {
    ($bytes:expr, $size:expr, $index:expr) => {
        $bytes[$index] == b'f'
            && $index + 4 < $size
            && $bytes[$index + 1] == b'a'
            && $bytes[$index + 2] == b'l'
            && $bytes[$index + 3] == b's'
            && $bytes[$index + 4] == b'e'
    };
}

macro_rules! maybe_number {
    ($b:expr) => {
        match $b {
            b'-' | b'0' | b'1' | b'2' | b'3' | b'4' | b'5' | b'6' | b'7' | b'8' | b'9' => true,
            _ => false,
        }
    };
}

#[derive(Debug)]
pub struct Error {
    index: usize,
    line: usize,
    column: usize,
    message: String,
}

impl Error {
    pub fn index(&self) -> usize {
        self.index
    }

    pub fn message(&self) -> &String {
        &self.message
    }

    fn simple(index: usize, message: String) -> Self {
        Error {
            index,
            line: 0,
            column: 0,
            message,
        }
    }

    fn unknown() -> Self {
        Error {
            index: 0,
            line: 0,
            column: 0,
            message: String::from("Unknown error"),
        }
    }

    fn empty() -> Self {
        Error {
            index: 0,
            line: 0,
            column: 0,
            message: String::from("Expected a value, but the input was empty"),
        }
    }

    fn expecting(
        valid: &[&str],
        got: u8,
        reason: &str,
        index: usize,
        line: usize,
        column: usize,
    ) -> Self {
        Error {
            index,
            line,
            column,
            message: format!(
                "Unexpected {}, expecting {} {reason}",
                format_byte!(got),
                match valid.len() {
                    0 => panic!("Error::expecting must be provided at least 1 valid choice"),
                    1 => format!("'{}'", valid[0]),
                    2 => format!("'{}' or '{}'", valid[0], valid[1]),
                    total => {
                        let mut list = format!("'{}'", valid[0]);
                        for (i, c) in valid.iter().skip(1).enumerate() {
                            list =
                                format!("{list}, {}'{c}'", if i == total - 1 { "or " } else { "" });
                        }
                        list
                    }
                }
            ),
        }
    }

    fn bad_state(state: State, reason: &str, index: usize, line: usize, column: usize) -> Self {
        Error {
            index,
            line,
            column,
            message: format!("Internal parsing error, invalid state {state:?} while {reason}"),
        }
    }

    fn string_needs_quotes(index: usize) -> Self {
        Self::simple(
            index,
            String::from("Strings must start and end with double quotes (\")"),
        )
    }

    fn string_escape_control(index: usize) -> Self {
        Self::simple(index, String::from("Control characters must be escaped"))
    }

    fn string_incomplete_u_escape(index: usize) -> Self {
        Self::simple(index, String::from("Incomplete \\uXXXX escape sequence"))
    }

    fn string_invalid_utf8_byte(index: usize, byte: u8) -> Self {
        Self::simple(
            index,
            format!("Ill-formed UTF-8 sequence, byte ({byte:?}) is not acceptable here"),
        )
    }

    fn string_incomplete_utf8_sequence(index: usize) -> Self {
        Self::simple(
            index,
            format!("Incomplete UTF-8 byte sequence, expected a trailing byte here"),
        )
    }

    fn number_incomplete(index: usize) -> Self {
        Self::simple(index, format!("Incomplete number, expecting a digit here"))
    }
}

pub fn validate(input: &str) -> Result<(), Error> {
    let bytes = input.as_bytes();
    let size = bytes.len();

    if size == 0 {
        return Err(Error::empty());
    }

    let mut next: usize = 0;

    // Skip over byte order mark
    if size >= 3 && is_byte_order_mark!(bytes) {
        if size == 3 {
            return Err(Error::empty());
        }
        next = 3;
    }

    use State::*;
    let mut state = VALUE;

    use StructureType::*;
    let mut stack: Vec<StructureType> = Vec::new();
    let mut b: u8;

    let mut line: usize = 1;
    let mut column: usize = 0;

    'outer: loop {
        'next_token: loop {
            b = bytes[next];
            next += 1;
            column += 1;

            if is_whitespace!(b) {
                if next >= size {
                    break 'outer;
                }
                if b == b'\n' {
                    line += 1;
                    column = 0;
                }
                continue 'next_token;
            }

            break 'next_token;
        }

        match state {
            // Immediately closed array
            ARRAY_START if b == b']' => {
                pop_stack!(stack, state);
            }
            // Values
            VALUE | ARRAY_START | ARRAY_VALUE | OBJECT_VALUE => {
                if b == b'[' {
                    state = ARRAY_START;
                    stack.push(ARRAY);
                } else if b == b'{' {
                    state = OBJECT_START;
                    stack.push(OBJECT);
                } else if b == b'"' {
                    let length = match validate_string(&bytes[next - 1..]) {
                        Ok(l) => l,
                        Err(mut error) => {
                            error.index += next - 1;
                            error.line = line;
                            error.column = column;
                            return Err(error);
                        }
                    };

                    next_value!(
                        state,
                        Err(Error::bad_state(
                            state,
                            "reading string",
                            next - 1,
                            line,
                            column
                        ))
                    );

                    next += length + 1; // "|str"
                } else if maybe_number!(b) {
                    let length = match validate_number(&bytes[next - 1..]) {
                        Ok(l) => l,
                        Err(mut error) => {
                            error.index += next - 1;
                            error.line = line;
                            error.column = column;
                            return Err(error);
                        }
                    };

                    next_value!(
                        state,
                        Err(Error::bad_state(
                            state,
                            "reading number",
                            next - 1,
                            line,
                            column
                        ))
                    );

                    next += length - 1; // 1|23
                } else if is_true!(bytes, size, next - 1) {
                    next_value!(
                        state,
                        Err(Error::bad_state(
                            state,
                            "reading literal 'true'",
                            next - 1,
                            line,
                            column
                        ))
                    );

                    next += 3; // t|rue
                } else if is_false!(bytes, size, next - 1) {
                    next_value!(
                        state,
                        Err(Error::bad_state(
                            state,
                            "reading literal 'false'",
                            next - 1,
                            line,
                            column
                        ))
                    );

                    next += 4; // f|alse
                } else if is_null!(bytes, size, next - 1) {
                    next_value!(
                        state,
                        Err(Error::bad_state(
                            state,
                            "reading literal 'null'",
                            next - 1,
                            line,
                            column
                        ))
                    );

                    next += 3; // n|ull
                } else {
                    return Err(Error::expecting(
                        &["{", "[", "\"", "-", "0-9", "true", "false", "null"],
                        b,
                        "value",
                        next - 1,
                        line,
                        column,
                    ));
                }
            }
            // Closing array
            ARRAY_END if b == b']' => {
                pop_stack!(stack, state);
            }
            // Continue array
            ARRAY_END if b == b',' => {
                state = ARRAY_VALUE;
            }
            // ERROR
            ARRAY_END => {
                return Err(Error::expecting(
                    &["]", ","],
                    b,
                    "for array",
                    next - 1,
                    line,
                    column,
                ));
            }
            // Immediately closed object
            OBJECT_START if b == b'}' => {
                pop_stack!(stack, state);
            }
            // Object key
            OBJECT_START | OBJECT_KEY if b == b'"' => {
                let length = match validate_string(&bytes[next - 1..]) {
                    Ok(l) => l,
                    Err(mut error) => {
                        error.index += next;
                        error.line = line;
                        error.column = column;
                        return Err(error);
                    }
                };
                next += length + 1;
                state = OBJECT_COLON;
            }
            // Colon after object key
            OBJECT_COLON if b == b':' => {
                state = OBJECT_VALUE;
            }
            // Closing object
            OBJECT_END if b == b'}' => {
                pop_stack!(stack, state);
            }
            // Continuing to next object key
            OBJECT_END if b == b',' => {
                state = OBJECT_KEY;
            }
            // ERROR
            OBJECT_START | OBJECT_KEY | OBJECT_COLON | OBJECT_END => {
                let (reason, valid) = match state {
                    OBJECT_START => ("for object", vec!["}", "\""]),
                    OBJECT_KEY => ("for object key", vec!["\""]),
                    OBJECT_COLON => ("following object key", vec![":"]),
                    _ => ("following object value", vec!["}", ","]),
                };

                return Err(Error::expecting(
                    valid.as_slice(),
                    b,
                    reason,
                    next - 1,
                    line,
                    column,
                ));
            }
            END if is_whitespace!(b) => {
                break 'outer;
            }
            // JSON is a single value surrounded by whitespace
            END => {
                return Err(Error {
                    index: next - 1,
                    line,
                    column,
                    message: format!(
                        "Extra {} at the end of the input, only one value is allowed",
                        format_byte!(b)
                    ),
                });
            }
        }

        if next >= size {
            break 'outer;
        }
    }

    if stack.is_empty() && state == END {
        Ok(())
    } else {
        Err(Error {
            index: next - 1,
            line,
            column,
            message: format!(
                "Unexpected end of input while reading {}",
                match state {
                    VALUE => "value",
                    ARRAY_START | ARRAY_END | ARRAY_VALUE => "array",
                    OBJECT_START | OBJECT_END | OBJECT_KEY | OBJECT_COLON | OBJECT_VALUE =>
                        "object",
                    END => {
                        match stack
                            .last()
                            .expect("Empty stack with state = END is not logically possible here")
                        {
                            ARRAY => "array",
                            OBJECT => "object",
                        }
                    }
                }
            ),
        })
    }
}

pub fn optimistic(input: &str) -> Option<Value> {
    Option::Some(Null())
}

fn validate_string(bytes: &[u8]) -> Result<usize, Error> {
    let size = bytes.len();
    if size < 2 {
        return Err(Error::string_needs_quotes(0));
    }

    if bytes[0] != b'"' {
        return Err(Error::string_needs_quotes(0));
    }

    let mut i: usize = 1;
    loop {
        let b = bytes[i];
        if b == b'"' {
            break;
        }

        // Guard early for unterminated string
        if i + 1 >= size {
            return Err(Error::string_needs_quotes(i));
        }

        let skip: usize = match b {
            0x00..=0x1F => {
                return Err(Error::string_escape_control(i));
            }
            0x20..=0x5B | 0x5D..=0x7F => 1, // Valid ascii
            b'\\' if i + 1 < size => {
                let b = bytes[i + 1];
                match b {
                    b'"' | b'\\' | b'/' | b'b' | b'f' | b'n' | b'r' | b't' => 2, // Valid
                    b'u' => match validate_escape_sequence(&bytes[i..]) {
                        Ok(skip) => skip,
                        Err(mut error) => {
                            error.index += i;
                            return Err(error);
                        }
                    },
                    _ => {
                        return Err(Error::simple(
                            i,
                            String::from(
                                "Invalid escape sequence. Only \\\", \\\\, \\/, \\b, \\f, \\n, \\r, \\t, and \\uXXXX are valid.",
                            ),
                        ));
                    }
                }
            }
            // Only get here if nothing follows the `\`
            b'\\' => {
                return Err(Error::simple(i, String::from("Incomplete escape sequence")));
            }
            // UTF-8 byte, must guarantee valid UTF-8 sequence
            _ => match validate_utf8_sequence(&bytes[i..]) {
                Ok(skip) => skip,
                Err(mut error) => {
                    error.index += i;
                    return Err(error);
                }
            },
        };

        i += skip;

        // If i is now equal to size, we are left with an unterminated string...
        if i >= size {
            return Err(Error::string_needs_quotes(i));
        }
    }

    Ok(i - 1) // don't include final (")
}

// This method assumes the first two bytes are '\' and 'u'
// It should only be used when that is true
fn validate_escape_sequence(bytes: &[u8]) -> Result<usize, Error> {
    // \uXXXX
    // or \uXXXX\uXXXX
    let size = bytes.len();
    if size < 6 {
        return Err(Error::string_incomplete_u_escape(0));
    }

    let decoded = match decode_hex(&bytes[2..=5]) {
        Ok(v) => v,
        Err(mut error) => {
            error.index += 2; // skip "\u"
            return Err(error);
        }
    };

    match decoded {
        // Early out if the high portion is already invalid. This range is reserved for low surrogates.
        0xDC00..=0xDFFF => Err(Error::simple(
            0,
            String::from(
                "Invalid escaped surrogate pair, initial low surrogates in \\uDC00..\\uDFFF are not allowed",
            ),
        )),
        // This value must be part of a UTF-16 surrogate pair
        0xD800..=0xDBFF if size >= 12 && bytes[6] == b'\\' && bytes[7] == b'u' => {
            match decode_hex(&bytes[8..=11]) {
                Ok(low_surrogate) => {
                    if (0xDC00..=0xDFFF).contains(&low_surrogate) {
                        Ok(12) // Valid
                    } else {
                        Err(Error::simple(
                            0,
                            String::from(
                                "Invalid escaped surrogate pair, low value is outside \\uDC00..\\uDFFF",
                            ),
                        ))
                    }
                }
                Err(mut error) => {
                    error.index += 8;
                    Err(error)
                }
            }
        }
        0xD800..=0xDBFF => Err(Error::simple(
            0,
            String::from(
                "Incomplete escaped surrogate pair. \\uD800..\\uDBFF sequences must be followed \
                 by another \\uXXXX escape sequence.",
            ),
        )),
        _ => Ok(6), // Anything else is valid
    }
}

fn validate_utf8_sequence(bytes: &[u8]) -> Result<usize, Error> {
    let size = bytes.len();
    if size < 1 {
        return Err(Error::simple(
            0,
            String::from("Internal error, zero bytes received in validate_utf8_sequence"),
        ));
    }

    // Lots of repetition in byte patterns, so macro the tails
    // NOTE the extra set of {} around the expression!
    macro_rules! is_valid_tail {
        ($i:expr) => {{
            if $i >= size {
                return Err(Error::string_incomplete_utf8_sequence($i));
            }
            match bytes[$i] {
                0x80..=0xBF => true,
                b => return Err(Error::string_invalid_utf8_byte($i, b)),
            }
        }};
        ($i:expr, $range:pat) => {{
            if $i >= size {
                return Err(Error::string_incomplete_utf8_sequence($i));
            }
            match bytes[$i] {
                $range => true,
                b => return Err(Error::string_invalid_utf8_byte($i, b)),
            }
        }};
    }

    let mut i: usize = 0;
    loop {
        // This follows Table 3-7 from the Unicode core-spec:
        // https://www.unicode.org/versions/Unicode17.0.0/core-spec/chapter-3/#G27506
        let skip: usize = match bytes[i] {
            0x00..=0x7F => break, // Stop processing when encountering ASCII bytes
            0xC2..=0xDF if is_valid_tail!(i + 1) => 2,
            0xE0 /*--*/ if is_valid_tail!(i + 1, 0xA0..=0xBF) && is_valid_tail!(i + 2) => 3,
            0xE1..=0xEC if is_valid_tail!(i + 1) /*--------*/ && is_valid_tail!(i + 2) => 3,
            // NOTE: Decoding sequences between EDA080..EDBFBF return D800..DFFF codepoints.
            //       These codepoints are reserved for UTF-16 surrogate pairs, and must
            //       written in escaped \uXXXX\uXXXX form for JSON.
            0xED /*--*/ if is_valid_tail!(i + 1, 0x80..=0x9F) && is_valid_tail!(i + 2) => 3,
            0xEE..=0xEF if is_valid_tail!(i + 1) /*--------*/ && is_valid_tail!(i + 2) => 3,
            0xF0 /*--*/ if is_valid_tail!(i + 1, 0x90..=0xBF) && is_valid_tail!(i + 2) && is_valid_tail!(i + 3) => 4,
            0xF1..=0xF3 if is_valid_tail!(i + 1) /*--------*/ && is_valid_tail!(i + 2) && is_valid_tail!(i + 3) => 4,
            0xF4 /*--*/ if is_valid_tail!(i + 1, 0x80..=0x8F) && is_valid_tail!(i + 2) && is_valid_tail!(i + 3) => 4,
            b => return Err(Error::string_invalid_utf8_byte(i, b)),
        };

        i += skip;

        if i >= size {
            break;
        }
    }

    Ok(i)
}

fn decode_hex(bytes: &[u8]) -> Result<u16, Error> {
    let mut result: u16 = 0;
    let mut i: usize = 0;

    for b in bytes {
        result = result.saturating_mul(16);

        match b {
            b'0' => continue,
            b'1'..=b'9' => {
                result += (b - b'0') as u16;
            }
            b'a'..=b'f' => {
                result += 10 + (b - b'a') as u16;
            }
            b'A'..=b'F' => {
                result += 10 + (b - b'A') as u16;
            }
            _ => {
                return Err(Error::simple(
                    i,
                    String::from("Invalid hex digit in escape sequence"),
                ));
            }
        }

        i += 1;
    }

    Ok(result)
}

fn validate_number(bytes: &[u8]) -> Result<usize, Error> {
    let mut i: usize = 0;
    let size = bytes.len();

    if size == 0 {
        return Err(Error::simple(
            0,
            String::from("Internal error, zero bytes received in validate_number"),
        ));
    }

    // Number validation is easier this way since the flow chart is very linear
    // with only three loops. No need to make it complicated out here, tracking
    // states and what-not.

    if bytes[i] == b'-' {
        i += 1;
        // There must be a digit after the '-'
        if i >= size {
            return Err(Error::number_incomplete(i));
        }
    }

    // JSON does not permit leading zeros, so consume digits at once
    match bytes[i] {
        b'0' => i += 1,
        b'1'..=b'9' => {
            i += 1;
            loop {
                if i >= size {
                    break;
                }
                match bytes[i] {
                    b'0'..=b'9' => i += 1,
                    _ => break,
                }
            }
        }
        _ => return Err(Error::number_incomplete(i)),
    }

    if i >= size {
        return Ok(i);
    }

    if bytes[i] == b'.' {
        i += 1;
        // There must be a digit after the '.'
        if i >= size {
            return Err(Error::number_incomplete(i));
        }
        match bytes[i] {
            b'0'..=b'9' => {
                i += 1;
                loop {
                    if i >= size {
                        break;
                    }
                    match bytes[i] {
                        // JSON does not care about trailing zeros...
                        b'0'..=b'9' => i += 1,
                        _ => break,
                    }
                }
            }
            _ => return Err(Error::number_incomplete(i)),
        }
    }

    if i >= size {
        return Ok(i);
    }

    if bytes[i] != b'e' && bytes[i] != b'E' {
        return Ok(i);
    }

    i += 1;

    // Must have a digit after 'e'
    if i >= size {
        return Err(Error::number_incomplete(i));
    }

    if bytes[i] == b'+' || bytes[i] == b'-' {
        i += 1;
        // Still need a digit
        if i >= size {
            return Err(Error::number_incomplete(i));
        }
    }

    // n e e d   d i g i t s
    if !(b'0'..=b'9').contains(&bytes[i]) {
        return Err(Error::number_incomplete(i));
    }

    i += 1;

    //   d  i    g    i t   s
    loop {
        if i >= size {
            break;
        }
        match bytes[i] {
            // JSON does not care about trailing zeros...
            b'0'..=b'9' => i += 1,
            _ => break,
        }
    }

    Ok(i)
}
