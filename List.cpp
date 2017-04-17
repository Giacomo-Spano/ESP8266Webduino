#include "stdafx.h"
#include "List.h"

#define String(x) (x)


List::List()
{
	init();
}

List::~List()
{
}

void List::init() {
	//logger.print(tag, "\n\t >>List::init");
	count = 0;
	first = nullptr;
	current = nullptr;
}

bool List::add(ObjectClass* elem) {

	if (first == nullptr) {
		first = elem;
		current = first;

	}
	else {

		current->next = elem;
		current = elem;
	}
	current->next = nullptr;
	count++;

	return true;
}

void List::clearAll() {

	//logger.print(tag, "\n\t >>List::clearAll");

	ObjectClass* next = first;
	ObjectClass* cur = first;
	while (cur != nullptr) {
		next = cur->next;
		delete cur;
		cur = next;
	}
	init();

	//logger.print(tag, "\n\t <<List::clearAll");
}

void List::show() {

	//logger.print(tag, "\n\t>> show");
	//logger.print(tag, "\n\t count = " + String(count));


	ObjectClass* p = first;
	int i = 0;
	while (p != nullptr) {
		i++;
		//logger.print(tag, "\n\t Elem=");
		//logger.print(tag, i);
		p->show();
		p = p->next;
	}
	//logger.print(tag, "\n\t <<show");
}

ObjectClass* List::gestLast() {

	current = first;
	while (current != nullptr && current->next != nullptr)
		current = current->next;

	return current;
}

ObjectClass* List::get(int n) {

	ObjectClass* p = first;
	for (int i = 0; i < count; i++) {
		if (i == n)
			return p;
		else
			p = p->next;
	}
	return nullptr;
}

int List::length() {

	return count;
}

bool List::hasNext() {

	if (current->next != nullptr)
		return true;
	else
		return false;
}

ObjectClass* List::getNext() {

	if (current->next != nullptr) {
		current = current->next;
		return current;
	}
	else
		return nullptr;
}

ObjectClass* List::getFirst() {

	current = first;
	return current;
}
