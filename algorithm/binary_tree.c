#include "binary_tree.h"

/*!
 * Функция выделения памяти под один объект структуры tree.
 *
 * @return указатель на tree.
 */
tree *createTree() {
    tree *t = (tree *) (malloc(sizeof(tree)));
    t->root = NULL;
    t->nodes_count = 0;
    return t;
}

/*!
 * Функция, добавления узла на ветку.
 *
 * Сначала проверяем, есть ли у `current_node` левый ребенок. Если нет, то вешаемся туда. Если есть, то проверяем
 * встречается ли new_node чаще, чем левый ребенок current_node. Если да, то вешаем узел на правую ветку от
 * current_node. Иначе, вешаем налево.
 *
 * @param current_node[in]: Узел, на который мы добавляем new_node;
 * @param new_node[in]: Узел, который мы добавляем.
 */
void m_addNode(node *current_node, node *new_node) {
    if (current_node->left == NULL) {
        current_node = new_node;
    } else {
        if (current_node->left->frequency <= new_node->frequency) {
            current_node->right = new_node;
        } else {
            current_node->right = current_node->left;
            current_node->left = new_node;
        }
    }
    current_node->frequency += new_node->frequency;
}

/*!
 * Обертка над функцией `m_addNode(node* current_node, node* new_node)`, сделанная просто для удобства.
 *
 * @param t[in]: Дерево
 * @param new_node[in]: Узел для добавления.
 */
void addNode(tree *t, node *new_node) {
    m_addNode(t->root, new_node);
}