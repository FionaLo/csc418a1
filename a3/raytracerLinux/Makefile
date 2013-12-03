CC = g++ 
CFLAGS = -g -O2 
LIBS = -lm -lpthread

raytracer:  raytracer.o util.o light_source.o scene_object.o bmp_io.o cylinder.o area_light_source.o
	$(CC) $(CFLAGS) -o raytracer \
	raytracer.o util.o light_source.o scene_object.o bmp_io.o cylinder.o area_light_source.o $(LIBS)

clean:
	-rm -f core *.o
	-rm raytracer
	



