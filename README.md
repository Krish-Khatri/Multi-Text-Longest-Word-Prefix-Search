Git Repository for project files

## All commands and code have been tested

- Set up JAVA_HOME environment variable

`export JAVA_HOME=/usr/java/latest`

- Compile java files in both packages

`javac edu/cs300/*java`

`javac CtCILibrary/*.java`

- Create header file for System V C message functions

`javac -h . edu/cs300/MessageJNI.java`

- Compile C native functions into a library for use with MessageJNI

`gcc -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux system5_msg.c -o edu_cs300_MessageJNI.o`

`gcc -shared -o libsystem5msg.so edu_cs300_MessageJNI.o -lc`

- Compile test send and receive functions

`gcc -std=c99 -D_GNU_SOURCE msgsnd_pr.c -o msgsnd`

`gcc -std=c99 -D_GNU_SOURCE msgrcv_lwr.c -o msgrcv`


## Commands to run sample programs

- Create a message with the prefix and ID and puts it on a queue
`./msgsnd con`

- Java program reads queue contents using native C function and creates and sends a response message via the System V msg queue

`java -cp . -Djava.library.path=. edu.cs300.MessageJNI`

- Retrieves message from System V queue and prints it
`./msgrcv`

- Program to illustrate use of Java threading, Trie and BlockingArrayQUEUE
`java edu.cs300.ParallelTextSearch con`

## Commands to check/delete message queues

- Determine the status of your message queue via `ipcs -a`
- Removes the queue along with any messages `ipcrm -Q 0x0c030904`

