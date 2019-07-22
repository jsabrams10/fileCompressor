all: fileCompressor

fileCompressor: fileCompressor.c
	gcc -o fileCompressor fileCompressor.c

clean:
	rm fileCompressor
