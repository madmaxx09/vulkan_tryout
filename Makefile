CFLAGS = -std=c++17 -g -Iimgui -Iimgui/backends #-O2 #fsanitize=address
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRC = $(wildcard *.cpp) \
      $(wildcard imgui/*.cpp) \
      $(wildcard imgui/backends/imgui_impl_vulkan.cpp) \
      $(wildcard imgui/backends/imgui_impl_glfw.cpp)

vulkanTest: $(SRC)
	bash compile.sh
	g++ $(CFLAGS) -o vulkanTest $(SRC) $(LDFLAGS)

.PHONY: test clean

test: vulkanTest
	./vulkanTest

clean:
	rm -f vulkanTest
