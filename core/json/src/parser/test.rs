use super::*;

#[test]
fn literal() -> Result<(), Error> {
    validate("true")?;
    validate("false")?;
    validate("null")?;

    Ok(())
}

#[test]
fn string() -> Result<(), Error> {
    validate(r#""""#)?; // Empty string
    validate(r#""hello there, this is a string""#)?;
    validate(r#""😀 2£ § どどど￿￿ 𐌀𐌋𐌑𐌆𐌂""#)?; // utf-8 spam
    validate(r#""\uD83C\udCa1""#)?; // escaped surrogates
    validate(r#""\u0000\u1234\u0032\uFfFf""#)?; // escaped random
    validate(r#""􏿿\uDBFF\uDFFF""#)?; // 10FFFF raw and 20FFFF escaped surrogate

    // This test isn't really about strings, it's only to test that a leading UTF-8 byte mark is ignored
    let leading_byte_mark =
        str::from_utf8(b"\xEF\xBB\xBF\"string\"").expect("Byte order mark is valid UTF-8");
    validate(leading_byte_mark)?;

    Ok(())
}

#[test]
fn number() -> Result<(), Error> {
    validate("0")?;
    validate("12")?;
    validate("-3")?;
    validate("1.0")?;
    validate("-1.0000")?;
    validate("2.87e-4")?;
    validate("1e2")?;
    validate("3.1415926589793")?;
    validate("1.618033988749894848204586834365638117720309179805762862135448622")?;
    validate("-0.0e-0")?; // yes this is a valid number
    validate("-1234.00876e+0000981803")?; // this one, too

    Ok(())
}

#[test]
fn whitespace() -> Result<(), Error> {
    validate("  {   }  ")?;
    validate("                                           \"sup\"                             ")?;
    validate("  \t1234\t   ")?;
    validate(
        "
\t\t\t\r\r\r\n\r\n{
            \t   \n  \t\n   \r\t    \r \"key\"  \t \r
    :           \t\r\n  \t
\r    [\t\"value\"\r\n
]   \t\t\t\t\t\t\t\t\t\t        }
    ",
    )?;

    Ok(())
}

#[test]
fn nested_structure() -> Result<(), Error> {
    validate(r#" {"":[{},[],{"hello":["world"]}], "another": 1} "#)?;

    validate(r#"[[[["deep", "stuff", {"interior": "crocodile"}]], 3.1415], {}, [21]]"#)?;

    validate(
        r#"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[{"secret":""}]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"#,
    )?;

    validate(
        r#"
{"":[[{"": {"":[[{"": {"":[[{"": {"":[[{"": {"":[[{"": {"":[[{"": {"":[[{"": {"":[[{"": {"":[[{"":
{"hello": "world"}
}]]} }]]} }]]} }]]} }]]} }]]} }]]} }]]} }]]}
"#,
    )?;

    // 1 thousand arrays
    validate(
        r#"
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
    "#,
    )?;

    Ok(())
}

#[test]
fn missing_value() {
    validate("").expect_err("Empty input should be invalid");

    // Taking a value, one-liner is a little confusing
    let lone_byte_mark = str::from_utf8(b"\xEF\xBB\xBF").expect("Byte order mark is valid UTF-8");
    validate(lone_byte_mark).expect_err("Byte order mark is not a value");

    validate("    ").expect_err("Whitespace only should be invalid");
    validate(" \n").expect_err("Whitespace only should be invalid");

    validate(r#"{"k": }"#).expect_err("Object keys must have values");
    validate("[0,]").expect_err("Arrays must have values after ','");
}

#[test]
fn incomplete_value() {
    validate("\"").expect_err("Lone double quote is invalid");
    validate(r#""unterminated string"#).expect_err("Incomplete strings are invalid");

    // Incomlete literals
    validate(r#"tru"#).expect_err("\"tru\" should be invalid");
    validate(r#"fals"#).expect_err("\"fals\" should be invalid");
    validate(r#"nul"#).expect_err("\"nul\" should be invalid");

    // Numbers must end with digits
    validate("-").expect_err("- should be invalid");
    validate("0.").expect_err("0. should be invalid");
    validate("1.").expect_err("1. should be invalid");
    validate("0.e").expect_err("0.e should be invalid");
    validate("1.e").expect_err("1.e should be invalid");
    validate("0.1e").expect_err("0.1e should be invalid");
    validate("1.0e").expect_err("1.0e should be invalid");
    validate("0.1e+").expect_err("0.1e+ should be invalid");
    validate("1.0e+").expect_err("1.0e+ should be invalid");
    validate("0.1e-").expect_err("0.1e- should be invalid");
    validate("1.0e-").expect_err("1.0e- should be invalid");
}

#[test]
fn invalid_value() {
    validate("text").expect_err("Regular text is invalid");

    validate("'single quotes'").expect_err("Single quotes are invalid");
    validate(r#"\u0032"#).expect_err("Escape sequences outside of strings should be invalid");
    validate(r#""\uDC1A""#).expect_err("Incomplete escaped surrogates are invalid");
    validate(r#""\uD900""#).expect_err("Incomplete escaped surrogates are invalid");
    // IDEA: Figure out how to provide invalid UTF-8 strings to the validation
    //       method. Rust normally prevents bad strings from existing, but I
    //       would like to be sure the validator doesn't panic.

    validate("00").expect_err("00 should be invalid");
    validate("000").expect_err("000 should be invalid");
    validate("001").expect_err("001 should be invalid");
    validate("00.1").expect_err("00.1 should be invalid");
    validate("+0").expect_err("Leading '+' is invalid");
    validate("-+1").expect_err("-+1 should be invalid");

    // These tests are probably pointless, but you never know...
    validate("()").expect_err("() should be invalid");
    validate("Ok()").expect_err("Ok() should be invalid");
    validate("Ok(())").expect_err("Ok(()) should be invalid");
    validate(r#"panic!("wow!");"#).expect_err("this is bad, real bad...");

    validate(r#""string"1234true"#).expect_err("Value series should be invalid");
    validate("false1234").expect_err("Value series should be invalid");

    validate("true,true").expect_err("Commas outside of arrays should be invalid");
    validate(r#""key":"value""#).expect_err("Colons outside of objects should be invalid");

    validate("{12,null}").expect_err("Objects should not be treated like arrays");
    validate(r#"["key":9]"#).expect_err("Arrays should not be treated like objects");
}
