# More About surface-json

Extra information. This file is not included in the public crate and is only available on GitHub. This is done to improve download speed and reduce slinging files most people don't need across the internet.


## FAQ

There are some deliberate implementation exclusions made that are addressed here.

### Why don't you make Short String Optimizations (SSOs)?

    Libraries should be lean and as simple to use as possible. Strings parsed from JSON should always provide Strings to the user. SSOs are not implemented in pursuit of this goal. Otherwise, you have to write more code every time you have to deal with strings.

    If you are using this library to specifically parse sequences of short strings, and performance is mission critical, stop using JSON.

### Why don't you have a custom Number type to represent huge numbers?

    JSON is an interoperable format, and parsers are relatively easy to implement in any language. The majority of devices are capable of 64-bit floating precision. Any legitimate specification for JSON points out these facts, and recognize that expecting parsers and generators to support arbitrarily high precision goes against the core purpose of JSON.

    Anyone already providing large numbers over JSON is already using alternative formats, like Base64 strings, because they know parsers don't all handle numbers the same way. Use an alternative format, or stop using JSON.

### Why don't you have X feature?

    I do plan to add additional validation features, such as requiring the value to be a specific type, limiting nesting depth, or objects to require specific keys. This can save processing time if the validation can stop earlier when basic restrictions aren't met.

    Other than that, anything not already in this JSON library most likely never will be. This is a parser intended to follow the JSON specification, and it has no business trying to do anything more. It would all just be gimmicks, more code to maintain, and undesired stuff for most developers who just wanted a dang HashMap.

    If you want anything this library doesn't provide, you would probably be happier just not using JSON anymore.

### Your parser is slow.

    If you can improve its overall performance, without sacrificing its robustness or simplicity, please contribute!

    Or, of course, stop using JSON and roll your own binary format.

### Your parser is insecure.

    If you mean that it doesn't have safe-guards for memory allocation or nesting, true! If it is something more serious, please tell me directly.

    I do plan to at least add a maximum depth option, but memory consumption is not applicable. It already validates that input is valid JSON before it tries to do any object construction (and that's already more than what most parsers do!)

    If you are passing in megabytes of JSON that is just nested objects carefully crafted to maximize memory consumption... don't do that! Rate limit those clients! Reject long input! Be serious here! Don't blame the bucket for the hole in your boat!

### Do you hate JSON?

    No, I love it. Some people can't appreciate the simplicity of it and want it (and parsers) to have every feature they can think of. But, JSON isn't for you or me, it is for everyone. Leave it alone and love it for what it is, not what you want it to be. JSON is simple. JSON is dependable.
