mod parser;

// Expose value types as top-level
pub use value::Value::{self, Array, Bool, Null, Number, Object};

// Expose error type and parse/ validate methods as top-level
pub use parser::{Error, parse, validate};

mod value {
    use std::collections::HashMap;

    pub enum Value {
        Null(),
        Bool(bool),
        Number(f64),
        String(String),
        Array(Vec<Value>),
        Object(HashMap<String, Value>),
    }
}
