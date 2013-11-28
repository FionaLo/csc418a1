CC = g++
CFLAGS = -g -O2 
LIBS = -lm

raytracer:  raytracer.o util.o light_source.o scene_object.o bmp_io.o cylinder.o
	$(CC) $(CFLAGS) -o raytracer \
	raytracer.o util.o light_source.o scene_object.o bmp_io.o cylinder.o $(LIBS)

clean:
	-rm -f core *.o
	-rm raytracer
	



