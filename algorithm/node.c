#include "node.h"

/*!
 * Функция выделения памяти под один объект структуры node.
 * Инициализирует поля letter и frequency переданными аргументами.
 *
 * @param letter[in]: символ, который кладем в node;
 * @param frequency[out]: частота, с которой этот символ встретился в тексте;
 * @return указатель на node.
 */
node *createNode(char letter, uint64_t frequency) {
    node *n = (node *) (malloc(sizeof(node)));
    n->letter = letter;
    n->frequency = frequency;
    n->left = NULL;
    n->right = NULL;
    return n;
}

/*!
 * Функция, которая меняет два указателя местами. Объекты, лежащие под ними остаются неизменными.
 *
 * @param a[out]
 * @param b[out]
 */
void swap(node *a, node *b) {
    node temp = *b;
    *b = *a;
    *a = temp;
}
