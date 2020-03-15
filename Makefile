.PHONY: build

build: so-cpp

so-cpp: so-cpp.c
	cl.exe so-cpp.c /MD

clean:
	del so-cpp.exe so-cpp.obj