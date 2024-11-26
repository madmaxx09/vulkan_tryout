CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

vulkanTest: *.cpp *.hpp
	bash compile.sh
	g++ $(CFLAGS) -o vulkanTest *.cpp $(LDFLAGS)

.PHONY: test clean

test: vulkanTest
	./vulkanTest

clean:
	rm -f vulkanTest
