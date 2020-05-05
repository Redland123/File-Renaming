#include "List.h"

void LL::add(std::filesystem::path newPath) {
	if (!this->head) {
		this->head = new node;
		this->head->path = newPath;
		this->head->next = NULL;
		this->tail = this->head;
	}
	else {
		this->tail->next = new node;
		this->tail->next->path = newPath;
		this->tail = this->tail->next;
		this->tail->next = NULL;
	}
	++(this->size);
}

//Deletes the entire list when called
LL::~LL() {
	if (!this->head) return;

	node* current = this->head;
	node* previous = this->head;

	while (current) {
		current = current->next;
		delete previous;
		previous = current;
	}
}

node::~node() {}
node::node() { this->next = nullptr; }

LL::LL() {
	this->head = nullptr;
	this->tail = nullptr;
	this->size = 0;
}
