#include "queue.h"
#include <string.h>

static Queue_t queues[QUEUES_COUNT];

//Функция инициализации очереди
//Входные параметры: указатель на очередь, указатель на mempool массив, максимальная емкость очереди, размер элемента)
//Выходные параметры: 
void queueInit(Queue_t* queue, void* storage, uint8_t capacity, uint8_t elementSize)
{
    //инициализируем массив нулями
    memset(storage, 0x00, elementSize*capacity);
    queue->capacity = capacity;
    queue->elementSize = elementSize;
    //первый элемент - не существует
    queue->first = (void*)0;
    //следующий элемент - нулевой элемент mempool
    queue->next = storage;   
    //указатель на нулевой элемент mempool
    queue->storage = storage;
}

//Функция получения указателя на очереди
//Входные параметры: можно сделать int и функция выбора из статик очередей
//Выходные параметры: указатель на очередь.
Queue_t * queueGetId(uint8_t id)
{
    assert_param(id <= QUEUES_COUNT - 1);
    return &queues[id];
}

//Функция записи в очередь
//Входные параметры: указатель на очередь, указатель на элемент
//Выходные параметры: записалось или нет
bool queueEnqueue(Queue_t* queue, void * element)
{
    //если очередь заполнена
	if (queue->first == queue->next)
    {
		return false;
    }
    //записываем сообщение
	memmove((void*)queue->next, (void*)element, queue->elementSize);
    
    //если в очереди было пусто
    if (queue->first == (void*)0)
    {
        queue->first = queue->next;
    }
    
    //если указатель на следующий элемент выходит за границы mempool
    if ((uint8_t *)queue->next + queue->elementSize == (uint8_t *)queue->storage + (queue->elementSize*queue->capacity))
    {
        //сбрасываем указатель, замыкаем кольцо
        queue->next = queue->storage;
    }        
    else
    {
        queue->next = (uint8_t*)queue->next + queue->elementSize;
    }
    return true;
}

//Функция вынимания элемента из очереди
//Входные параметры: указатель на очередь, указатель на элемент
//Выходные параметры: есть данные или нет
bool queueDequeue(Queue_t* queue, void * element)
{
    //если очередь пустая
	if (queue->first == (void*)0)
    {
		return false;
    }
	//отдаем сообщение наружу
	memmove((void*)element, (void*)queue->first, queue->elementSize);
    //если указатель на текущее сообщение вышел за пределы mempool
    if ((uint8_t *)queue->first + queue->elementSize == (uint8_t *)queue->storage + (queue->elementSize*queue->capacity))
    {
        //обнуляем указатель, замыкаем кольцо
        queue->first = queue->storage;
    }        
    else
    {
        queue->first = (uint8_t *)queue->first + queue->elementSize;
    }
    //если мы вычитали последний элемент
    if (queue->next == queue->first)
    {
        //сбрасываем указатель
        queue->first = (void*)0;
    }
   	return true;
}

//Функция чтение элемента очереди без вынимания
//Входные параметры: указатель на очередь, указатель на элемент
//Выходные параметры: есть данные или нет
bool queuePeek(Queue_t* queue, void * element)
{
	if (queue->first == (void*)0)
    {
		return false;
    }
	memmove((void*)element, (void*)queue->first, queue->elementSize);
	return true;
}

//Функция вычисления заполненности очереди.
//Входные параметры: указатель на очередь
//Выходные параметры: есть данные или нет
uint8_t queueSize(Queue_t * queue)
{	
    uint8_t size;
    
    if (queue->first == (void*)0)
    {
        size = 0;
    }
    else if (queue->first < queue->next)
    {
        size = ((uint32_t)queue->next - (uint32_t)queue->first) / queue->elementSize;
    }
    else
    {
        size = queue->capacity - (((uint32_t)queue->first - (uint32_t)queue->next) / queue->elementSize);   
    }
    return size;
}

//Функция вычисления заполненности очереди.
//Входные параметры: указатель на очередь
//Выходные параметры: заполнена ли очередь
bool queueIsFull(Queue_t * queue)
{	
    return queue->first == queue->next;
}

//Функция вычисления заполненности очереди.
//Входные параметры: указатель на очередь
//Выходные параметры: пустая ли очередь
bool queueIsEmpty(Queue_t * queue)
{	
     return queue->first==(void*)0;
}

