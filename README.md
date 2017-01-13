Group Members:
John Spinelli
Mike Haley
Clayton Ezzell

Webserver

-Our webserver is able to pass all test cases presented in the assignment. The following use cases indicate the procedure needed to run our code.

Use Cases

-To run our webserver within the terminal, type ./webserver xxxx, where xxxx represents a port number of your choice. The server can then be found on any browser by typing “localhost:xxxx/“ in the url, followed by the request you choose to make. 

-To run my-histogram, type “localhost:xxxx/my-histogram?directory=param1”, where param1 is the root directory from which you wish to count files. The output is directed into a .dat file which gnuplot uses to generate a histogram. The histogram is then embedded in an html file which we cat onto the browser. 

-arduino:

Acknowledgements:

-As mentioned in our comments, the code used to traverse directories to obtain a count of files is taken from our course textbook’s source code and our previous code from assignment one. Our gnuplot code follows the multiple examples for generating histograms found at gnuplot.sourceforge.net/docs. 

-As a side note, we’d like to thank you as a collective for one of the most challenging yet rewarding semesters of our college careers. Till next time. 
