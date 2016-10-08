.SILENT:

default:
	gcc -o lab0 -g lab0.c
check: default all
	if [[ -s log.txt ]]; then \
	echo "One of the Tests has Failed"; \
	cat log.txt; \
	rm -f log.txt; \
	else \
		echo "Tests successful"; \
	fi
clean:
	rm -f lab0 *.txt
dist: default	
	tar -czf lab0-004480345.tar.gz lab0.c Makefile README backtrace.png null.png
	
ioTest:
	echo "test123test123" > input.txt; \
	./lab0 --input=input.txt --output=output.txt; \
	if [[ $$? -ne 0 ]]; then \
	echo "Error: A problem occurred with the input and output files" >> log.txt; \
	fi; \
	rm -f input.txt output.txt
differentTest:
	echo "\n\n123@@@&" > input.txt; \
	./lab0 --input=input.txt --output=output.txt; \
	cmp input.txt output.txt;\
	if [[ $$? -ne 0 ]]; then \
	echo "Error: The input was not correctly copied to the output" >> log.txt; \
	fi; \
	rm -f input.txt output.txt
readTest:
	echo "\n\n123@@@&" > input.txt; \
	chmod 333 input.txt; \
	echo | ./lab0 --input input.txt &> /dev/null; \
	if [[ $$? -ne 1 ]]; then \
	echo "Error: Exit Status 1 not returned" >> log.txt; \
	fi; \
	rm -f input.txt
writeTest:
	echo "\n\n123@@@&" > input.txt; \
	touch output.txt; \
	chmod 555 output.txt; \
	./lab0 --input input.txt --output output.txt &> /dev/null; \
	if [[ $$? -ne 2 ]]; then \
	echo "Error: Exit Status 2 not returned" >> log.txt; \
	fi; \
	rm -f input.txt output.txt
segfaultTest:
	echo | ./lab0 --segfault --catch &> /dev/null; \
	if [[ $$? -ne 3 ]]; then \
	echo "Error: Segfault not caught" >> log.txt; \
	fi
all: ioTest differentTest readTest writeTest segfaultTest
