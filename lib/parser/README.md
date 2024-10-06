# Parsers
 1. JSON
 2. Key-Value
 3. Multipart
 4. Http Requests

## Policy for deciding parser strictness.
It depends but, let's define some rules to follow:
1. Does specification require it?
    - if ambiguous (not-strictly defined), prioritize as follows:
        - performance
        - simplicity
        - strictness
2. Generalize create and use abstractions only as much as it doesn't affect performance.
3. Be aware non-strict to strict might be part of the process. #pro-upgrades.
