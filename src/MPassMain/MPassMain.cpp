#include <Common/MPassPch.h>

#include <Buffers/BufferAllocator.h>

int main(int argc, const char * argv[])
{
    MPass::Buffers::BufferAllocator allocator(1, 1);
	std::cout << "Hello World" << std::endl;
	return 0;
}