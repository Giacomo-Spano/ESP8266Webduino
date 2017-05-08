#include "stdafx.h"
#include "List.h"

//#define String(x) (x)

Logger List::logger;
String List::tag = "List";

List::List()
{
	count = 0;
	first = nullptr;
	current = nullptr;
	//init();
}

List::~List()
{
	logger.println(tag, ">>~List");
	//clearAll();
	logger.println(tag, "<<~List");
}

void List::init() {
	/*logger.println(tag, ">>init");
	count = 0;
	first = nullptr;
	current = nullptr;
	logger.println(tag, "<<init");*/
}

bool List::add(ObjectClass* elem) {

	//logger.println(tag, "\t adding " + elem->toString());

	if (elem == nullptr) {
		logger.print(tag, "\t ELEM null !!");
		return false;
	}
	
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

	//logger.print(tag, "\n" + elem->toString() + " added, count=" + String(count));
	//logger.print(tag, "\n");
	return true;
}

void List::clearAll() {

	logger.println(tag, ">>clearAll count=" + String(count) + "\n");

	ObjectClass* next = first;
	ObjectClass* cur = first;
	while (cur != nullptr && count > 0) {

		logger.print(tag, "\t record " + cur->toString());
		next = cur->next;
		delete cur;
		cur = next;
		logger.print(tag, " deleted\n");
	}
	count = 0;
	first = nullptr;
	current = nullptr;

	logger.println(tag, "<<clearAll");
}

void List::show() {

	logger.print(tag, "\n");
	logger.println(tag, ">>show count = " + String(count));


	ObjectClass* p = first;
	int i = 0;
	while (p != nullptr) {
		i++;
		//logger.print(tag, "\n\t Elem=");
		logger.print(tag, "key = " + p->toString() + "\n");
		//p->show();
		p = p->next;
	}
	logger.println(tag, "<<show");
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
