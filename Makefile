.PHONY: clean

helloworld: out.o
	gcc -o helloworld out.o
	rm -rf out.o

out.o: helloworld.ad
	./build/abheek_lang helloworld.ad

clean:
	rm -f out.o
