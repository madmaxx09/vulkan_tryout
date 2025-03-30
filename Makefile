CFLAGS = -std=c++17 -O2 -g -fsanitize=address
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRC = $(wildcard *.cpp) $(wildcard imgui/*.cpp) $(wildcard imgui/backends/imgui_impl_vulkan.*)
vulkanTest: $(SRC)
	bash compile.sh
	g++ $(CFLAGS) -o vulkanTest $(SRC) $(LDFLAGS)


.PHONY: test clean

test: vulkanTest
	./vulkanTest

clean:
	rm -f vulkanTest
