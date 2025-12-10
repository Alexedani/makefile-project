# Project 2 

- Name: Alex Daniluc
- Email: alexanderdaniluc@u.boisestate.edu
- Class: 452-002

## Known Bugs or Issues

TODO: 

There were 2 warnings I was getting that had to do with type conversion which is something im still not very familiar with in C

## Experience

A lot of this project was reviewing and remembering concepts from cs253 in regards to programming in C. For the most part, the project was pretty straight forward and a good learning experience for working with threads. For the parts which I was not able to understand, I was able to use resources online such as youtube and generative AI to help me.


# Project 5 - REDEMTION

## Things I've added to improve upon the project from my previous attempt on this assignment:

### Main.C Code Quality

I scored a 12/15 on this previously because I forgot to annotate my functions in Main.C and specify whether they were generated with AI or not. This was something that I went back and fixed for main.c

### Address Sanitzer Report 

I scored a 0/10 on this previously because there was a memory leak issue being caused that I had not addressed

Issue:
The program leaked memory during the merge step.
The original merge() function created a new output list but did not properly free the two sublists (left and right).
Additionally, after the merge, the main program never destroyed those sublists.

### Build Output - makeall

I had scored a 5/10 on this previously because I had two warnings that were being given when makeall was ran which had to do with type conversion. I have addressed these issues in main.c and there are no more warnings given.

### Threading

I had scored a 15/20 on this previously because the code was able to run, but was not producing the expected results. 

Fixing the memory leak issue now fixes the program and it is able to print the expected results of the sorted list/sample.

