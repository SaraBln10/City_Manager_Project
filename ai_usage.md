# AI Usage Documentation — Phase 1

## Tool Utilized  
Claude (claude-sonnet-4-6), accessed through claude.ai  

---

## Prompt 1 — parse_condition

**Request I made:**  
> I have a C struct named Report with these fields:  
> - id (int)  
> - inspector (char[64])  
> - latitude (double)  
> - longitude (double)  
> - category (char[32])  
> - severity (int)  
> - timestamp (time_t)  
> - description (char[256])  
>  
> Create a function:  
>   int parse_condition(const char *input, char *field, char *op, char *value);  
> that separates a string formatted as field:operator:value into its three components.  
> Valid operators include ==, !=, <, <=, >, >=.  

**AI-generated result:**  
The AI produced a function that relied on `strtok()` to divide the input string using `:` as a delimiter. It also correctly handled multi-character operators such as `<=` and `>=` by treating them as strings rather than individual characters.  

**Adjustments I made:**  
Because `strtok()` alters the original string by inserting null terminators, it is not safe to use when working with input from `argv[]`, which should remain unchanged. To address this, I rewrote the function using pointer arithmetic along with `strchr()`. This approach allows locating delimiter positions without modifying the original input string.  

---

## Prompt 2 — match_condition

**Request I made:**  
> Using the same Report struct, generate a function:  
>   int match_condition(Report *r, const char *field, const char *op, const char *value);  
> that returns 1 if a record meets the specified condition and 0 otherwise.  
> Supported fields: severity, category, inspector, timestamp.  
> Supported operators: ==, !=, <, <=, >, >=.  

**AI-generated result:**  
The AI created a function that properly separated logic for string-based fields (category, inspector) and numeric fields (severity, timestamp). It used `strcmp()` for comparing strings and `atoi()` to convert value strings into integers for numeric comparisons.  

**Adjustments I made:**  
The generated code used `atoi()` for handling the timestamp field. However, since `time_t` is usually a 64-bit value on modern Linux systems, using `atoi()` (which returns an `int`) could lead to truncation for large values. I replaced it with `atol()` to safely support the full range of Unix timestamps.  

Additionally, I introduced `fprintf(stderr, ...)` messages to warn about unknown fields or unsupported operators. The original version silently returned 0 in such cases, which would make debugging unnecessarily difficult.  

---

## Code written independently

The `filter_reports()` function was implemented entirely without AI assistance, in line with the assignment requirements. This function:  
- Opens `reports.dat` using `open()`  
- Reads each record sequentially with `read()`  
- Calls `parse_condition()` at the beginning for every condition to detect formatting issues early  
- Evaluates each record against all conditions using `match_condition()`  
- Outputs only the records that satisfy all conditions (logical AND behavior)  

---

## Key takeaways

- `strtok()` modifies its input by inserting null terminators, making it unsuitable for read-only data such as `argv[]`. Using pointer-based methods like `strchr()` is a safer alternative.  
- On 64-bit Linux systems, `time_t` is typically defined as a `long`, so using `atoi()` (which returns `int`) can result in truncation, especially for timestamps beyond 2038.  
- While AI-generated code can be largely correct for clearly defined tasks, it still requires careful inspection, particularly for edge cases and implementation specifics.  
- Reviewing the generated code thoroughly, line by line, proved more beneficial than simply using it as-is, as it ensured a deeper understanding of every design and implementation choice.  