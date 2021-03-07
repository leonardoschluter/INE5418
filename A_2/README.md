This software describe a  calculator application between a client and a server. 
Where the server executes basic math operations ( +, -, /, *)

The communication will happen in the following manner:
1. Client ask for a calculation passing three arguments ( operation, numberA, numberB ) in the form of a string operation#numberA#numberB.
1.1 Operation can only be these four chars: '+', '-', '*', '/'
1.2 NumberB e numberA must be integers.
2. The server receives the message and checks the format (includes checking operation format and number type)
2.1 If wrong format, respond "400: bad request Error: wrong message format"
3. If the format is correct, then server execute operation
3.1 And responds with "200 - Result: $value"
4. server reset waiting for a new calculation
5. client receives HTTP Code
5.1 interprets it and print the message then exits.
6. client resets and wait for another input.