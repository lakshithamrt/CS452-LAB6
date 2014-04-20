run: lab6.cpp 
	g++ lab6.cpp -lglut -lGLU -lGL -lGLEW -g 
	
clean: 
	rm -f *.out *~ run
	
