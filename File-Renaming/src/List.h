#include "leak.h"
#include <filesystem>

struct node {
	node();
	~node();
	std::filesystem::path path;
	node* next;
};

class LL {
public:
	LL();
	~LL();

	void add(std::filesystem::path newPath);

	unsigned int size;

	node* head;
	node* tail;
};
