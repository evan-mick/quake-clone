# quake-clone


How to run:

There are three ways to run this:

### With no command arguments: 
Running the file with no command arguments will run the game as a local instance

With command argument "s" will run the project without any graphics as a server instance

With command arguments "c <ip number>" the project will run with graphics and attempt to connect with a server on that IP
A connection will be established, and whatever is received on the server side will be rendered and the client will send its inputs as well as any objects created locally. 


This project uses GLEW and GLFW for OpenGL, make sure your system has these properly installed. 