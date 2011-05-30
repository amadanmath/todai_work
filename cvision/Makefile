PROJECT=reconstruct

${PROJECT}: ${PROJECT}.c
	gcc -O2 -o $@ $^

clean:
	rm -f *.o ${PROJECT} ${PROJECT}.exe points.txt dist-view.bmp
