#include "List.h"
#include "Logger.h"

extern Logger logger;


List::List()
{
	tag = "List";

	count = 0;
	first = nullptr;
	current = nullptr;
	//Serial.println("\nALIST CONSTRUCTOR ");
}


List::~List()
{
}

void List::init() {
	count = 0;
	first = nullptr;
	current = nullptr;
	/*Serial.println("\nINIT ");
	if (first == nullptr) {
		Serial.println("\nfirst IS NULLLLL");	
	}*/

}

/*bool List::prova(Sensor* elem){
	//Serial.println("\nADD NEW ELEMENT ");
	//Serial.print("\ncount = ");
	//Serial.println(count);

	if (first == nullptr) {
		//Serial.println("\nfirst ");
		first = elem;
		current = first;

	}
	//Serial.print("\ncount = ");
	//Serial.println(count);

	return true;
}*/

bool List::add(Sensor* elem) {
	
	//Serial.println("\nADD NEW ELEMENT ");
	//Serial.print("\ncount = ");
	//Serial.println(count);

	if (first == nullptr) {
		//Serial.println("\nfirst ");
		first = elem;
		current = first;
		
	} else {
				
		current->next = elem;
		current = elem;
		
	
	}
	current->next = nullptr;
	count++;
	//Serial.print("\ncount = ");
	//Serial.println(count);
		
	return true;
}

void List::show() {

	logger.print(tag, "\n\t>>show");
	logger.print(tag, "\n\tcount = ");
	logger.print(tag, count);

	/*if (first == nullptr) {
		logger.print(tag, "\n\tfirst NULL");
	} 
	else
		logger.print(tag, "\n\tfirst NOT NULL");*/

	
	Sensor* p = first;
	int i = 0;
	while (p != nullptr) {
		i++;
		logger.print(tag, "\n\tElem ");
		logger.print(tag, i);
		logger.print(tag, " name=");
		logger.print(tag, p->sensorname);
		logger.print(tag, " addr= ");
		logger.print(tag, p->getSensorAddress());
		p = p->next;
	}
	logger.print(tag, "\n\t<<show");
}

Sensor* List::gestLast() {

	current = first;
	while (current != nullptr && current->next != nullptr)
		current = current->next;

	return current;
}

Sensor* List::get(int n) {

	Sensor* p = first;
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

Sensor* List::getNext() {
		
	if (current->next != nullptr) {
		current = current->next;
		return current;
	}
	else
		return nullptr;
}

Sensor* List::getFirst() {

	current = first;
	return current;
}
