run: build 
	./hydrogen

data: build
	./hydrogen > docs/data.ts
	
build: 
	g++ -std=gnu++17 hydrogen.cpp -o hydrogen -L/usr/local/Cellar/ginac/1.7.9/lib -lginac -L/usr/local/Cellar/cln/1.3.6/lib -lcln