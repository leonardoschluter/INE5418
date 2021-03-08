This software describe a  calculator application between a client and a server. 
Where the server executes basic math operations ( +, -, /, *)

To install and run: 
first cd A_2 folder, then run `make all`
After that start the server in another terminal by /your_path/A_2/server/server
And then back to the original terminal start the client application by /your_path/A_2/client/application

The communication will happen in the following manner:
1. Client ask for a calculation passing three arguments ( operation, numberA, numberB ) in the form of a string operation#numberA#numberB.
1.1 Operation can only be these four chars: '+', '-', '*', '/'
1.1.1 Here you can specify char `s` in order to quit the application.
1.2 NumberB e numberA must be integers.
2. The server receives the message and checks the format (includes checking operation format and number type)
2.1 If wrong format, respond "wrong format message"
3. If the format is correct, then server execute operation
3.1 And responds with "$value"
4. server reset waiting for a new calculation
5. client receives result
5.1 interprets it and print the message if it is a calculation result
5.2 if it is the exit command, then exits.
6. client resets and wait for another input.