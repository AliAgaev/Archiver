#include "huffman_compressing.h"

/*!
 * Построение дерева из корня `root`, на основе кодов `huffmanCode`. Рекурсивная функция, использующая `str`
 * для построения кода.
 *
 * @param root[in]: корень дерева, в которое строим коды.
 * @param str[in]: текущий код при рекурсивном вызове.
 * @param huffmanCode[in]: таблица кодов Хаффмана.
 */
void encode(node *root, const char *str,
            char **huffmanCode) {
    if (root == NULL) {
        return;
    }

    if (!root->left && !root->right) {
        huffmanCode[(int) (root->letter)] = (char *) (malloc(sizeof(char) * strlen(str) + 1));
        strcpy(huffmanCode[(int) (root->letter)], str);
    }

    char *left = (char *) (malloc(sizeof(char) * strlen(str) + 2));
    strcpy(left, str);
    strcat(left, "0");
    encode(root->left, left, huffmanCode);
    free(left);

    char *right = (char *) (malloc(sizeof(char) * strlen(str) + 2));;
    strcpy(right, str);
    strcat(right, "1");
    encode(root->right, right, huffmanCode);
    free(right);
}

/*!
 * Декодирование символа и запись его в файл `f`. Файловый дескриптор `f` должен быть открыт при передачи в функцию,
 * иначе будет ошибка. `root`- корень дерева, где мы ищем код, расположенный в строке `str` по индексу `index`.
 *
 * @param root[in]: корень дерева, в которое строим коды.
 * @param str[in]: текущий код при рекурсивном вызове.
 * @param huffmanCode[in]: таблица кодов Хаффмана.
 */
void decode(node *root, int *index, const char *str, FILE* f) {
    if (root == NULL) {
        return;
    }

    if (!root->left && !root->right) {
        printf("%c", root->letter);
        fprintf(f, "%c", root->letter);
        return;
    }

    (*index)++;

    if (str[*index] == '0') {
        decode(root->left, index, str, f);
    } else {
        decode(root->right, index, str, f);
    }
}

/*!
 * Конвертация кода, записанного строкой в unsigned char (байт). Например, пускай code="101", то в результате мы
 * получим `unsigned char res = 0b00000101;`.
 *
 * @param code[in]: строковое представление двоичного кода.
 * @return вычисленный байт;
 */
unsigned char codeToByte(const char *code) {
    unsigned char res = 0;
    int len = strlen(code);
    for (int i = 0; i < len; ++i) {
        if (code[i] == '1') {
            res |= (1 << i);
        }
    }
    return res;
}

/*!
 * Построение дерева Хаффмана по заданному тексту. Алгоритм состоит в следующем:
 *      1. Считаем, с какой частотой встречается каждый символ в строке.
 *      2. Создаем бинарное дерево, на основе вычисленных частот, таким образом, что символы с наибольшей частотой
 *      будут иметь более короткий код, а коды с наименьшей частотой - длинный код.
 *      3. Записываем закодированные данные в файл.
 *          a) Сначала записываем `n` - количество уникальных символов (1 байт)
 *          b) Записываем таблицу символов: длина кода (1 байт), символ (1 байт), код (1 байт). Итого 3 байта под
 *          каждый из `n` символов. Повторяем этот шаг n раз. Например, если буква a имеет код 0110, то мы запишем
 *          такую последовательность байт: 00000100 | 01100001 | 00000110 (4 - длина | 'a' - символ | 0110 - код).
 *          с) Далее записываем сам закодированный текст.
 *          d) Дописываем padding - количество бит, чтобы файл имел целое количество байт.
 * @param string[in]: Текст6 который кодируем.
 */
void buildHuffmanTree(const char *string) {
    // Count frequency of characters: index - code of symbol, value - frequency of this symbol
    const int alphabet_size = (1 << (sizeof(char) * 8)); // 1 << sizeof(char) * 8 <=> 1 << 8 <=> pow(2, 8)
    uint64_t *frequency = (uint64_t *) (calloc(alphabet_size, sizeof(uint64_t)));
    for (uint64_t i = 0; i < strlen(string); ++i) {
        ++frequency[(int) (string[i])];
    }

    // Priority queue
    priority_queue *pq = createQueue();
    int unique_count = 0;
    for (int i = 0; i < alphabet_size; ++i) {
        if (frequency[i] > 0) {
            // priorityQueue Push
            insert(pq, createNode((char) (i), frequency[i]));
            ++unique_count;
        }
    }

    while (pq->size != 1) {
        node *left = createNode(pq->data[0]->letter, pq->data[0]->frequency);
        left->left = pq->data[0]->left;
        left->right = pq->data[0]->right;
        deleteRoot(pq, pq->data[0]->letter);

        node *right = createNode(pq->data[0]->letter, pq->data[0]->frequency);
        right->left = pq->data[0]->left;
        right->right = pq->data[0]->right;
        deleteRoot(pq, pq->data[0]->letter);

        uint64_t sum = left->frequency + right->frequency;

        node *new_node = createNode('\0', sum);
        new_node->left = left;
        new_node->right = right;

        insert(pq, new_node);
    }

    node *root = pq->data[0];

    char **huffmanCode = (char **) (calloc(alphabet_size + 1, sizeof(char *)));
    encode(root, "", huffmanCode);

//    printf("Huffman Codes are:\n");
    FILE *encodedFile = fopen("encoded.zip", "wb");
    if (encodedFile == NULL) {
        printf("encodedFile can not be opened.");
        return;
    }
    fwrite(&unique_count, 1, 1, encodedFile);
    for (int i = 0; i < alphabet_size; ++i) {
        if (huffmanCode[i] != NULL) {
            printf("%c: %s\n", (char) (i), huffmanCode[i]);
            unsigned char len = strlen(huffmanCode[i]);
            unsigned char symbol = (char) (i);
            unsigned char code = codeToByte(huffmanCode[i]);
            fwrite(&len, 1, 1, encodedFile);
            fwrite(&symbol, 1, 1, encodedFile);
            fwrite(&code, 1, 1, encodedFile);
        }
    }

//    printf("\nOriginal string was :\n%s\n", string);

//    printf("\nEncoded string is :\n");
    char *encoded = (char *) (calloc(1000000, sizeof(char)));
    for (int i = 0; i < strlen(string); ++i) {
        strcat(encoded, huffmanCode[(int) (string[i])]);
//        printf("%s", huffmanCode[(int)(string[i])]);
//        fprintf(encodedFile, "%s", huffmanCode[(int)(string[i])]);
    }
    int offset = 8 - strlen(encoded) % 8;
    fwrite(&offset, 1, 1, encodedFile);

    // write to binary
    for (int i = 0; i < strlen(encoded); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8 && i + j < strlen(encoded); ++j) {
            if (encoded[i + j] == '1') {
                byte |= (1 << j);
            }
        }
        fwrite(&byte, 1, 1, encodedFile);
    }
    fclose(encodedFile);


    free(encoded);
    free(frequency);
    deallocateQueue(pq);
}

/*!
 * Получение конкретного бита у байта.
 *
 * Например, если `byte = 0b00101110, pos = 3`, то мы получим 1, так как:
 *
 * 0b00101110
 *       ^
 *
 * Нумерация начинается с нуля и идет слева-направо.
 *
 * @param byte[in]: байт
 * @param pos[in]: номер бита, который хотим получить.
 * @return 1 или 0 как int.
 */
int getBit(unsigned char byte, int pos) {
    return (byte & (1 << pos)) ? 1 : 0;
}

/*!
 * Функция конвертации байта в десятичное число. Например, если `byte = 0b00000101`, то мы получим 5.
 *
 * @param byte[in]: двоичное представление байта.
 * @return десятичный вид байта.
 */
int binaryToDecimal(unsigned char byte) {
    int res = 0;
    for (int i = 0; i < 8; ++i) {
        res += (getBit(byte, i)) * (1 << i);
    }
    return res;
}

/*!
 * Функция, строящая дерево Хаффмана по закодированным символам. То есть обратное построение. Имея код `code` и
 * символ `letter` добавляем его в дерево `root`.
 *
 * @param root[in]: Корень дерева Хаффмана.
 * @param code[in]: Код символа.
 * @param letter[in]: Символ.
 * @param current_index[in]: Индекс символа кода, на который мы сейчас смотрим.
 */
void addNodeDecompress(node *root, const char *code, char letter, int current_index) {
    if (current_index + 1 == strlen(code)) {
        if (code[current_index] == '1') {
            root->right = createNode(letter, 1);
        } else {
            root->left = createNode(letter, 1);
        }
    } else {
        if (code[current_index] == '1') {
            if (root->right == NULL) {
                root->right = createNode('\0', 0);
            }
            addNodeDecompress(root->right, code, letter, current_index + 1);
        } else {
            if (root->left == NULL) {
                root->left = createNode('\0', 0);
            }
            addNodeDecompress(root->left, code, letter, current_index + 1);
        }
    }
}

/*!
 * Создание дерева по таблице кодов Хаффмана.
 * См. `void addNodeDecompress(node *root, const char *code, char letter, int current_index)`.
 *
 * @param huffmanCodes[in]: Таблица кодов, из которых мы строим дерево.
 * @return Возвращаем корень построенного дерева.
 */
node *createTreeFromCodes(char **huffmanCodes) {
    node *root = createNode('\0', 0);
    for (int i = 0; i < 256; ++i) {
        if (huffmanCodes[i] != NULL) {
            addNodeDecompress(root, huffmanCodes[i], (char) i, 0);
        }
    }
    return root;
}

/*!
 * Парсинг закодированного файла. Формат файла:
 * a) `n` - количество уникальных символов (1 байт)
 * b) Таблица символов: длина кода (1 байт), символ (1 байт), код (1 байт). Итого 3 байта под
 *    каждый из `n` символов. Повторяем этот шаг n раз. Например, если буква a имеет код 0110, то мы запишем
 *    такую последовательность байт: 00000100 | 01100001 | 00000110 (4 - длина | 'a' - символ | 0110 - код).
 * с) Сам закодированный текст.
 * d) `padding` - количество бит, чтобы файл имел целое количество байт.
 *
 * Создаёт файл `decompress.txt` - который является раскодированным `filepath`.
 *
 * @param filepath[in]: Путь к закодированному файлу.
 */
void parseFile(const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    const int alphabet_size = 256;
    char **huffmanCode = (char **) (calloc(alphabet_size + 1, sizeof(char *)));
    unsigned char byte;
    // read first byte
    fread(&byte, 1, 1, f);
    int unique_symbols_count = binaryToDecimal(byte);
    printf("Unique symbols: %d\n", unique_symbols_count);
    for (int i = 0; i < unique_symbols_count; ++i) {
        fread(&byte, 1, 1, f);
        int len = binaryToDecimal(byte);
        printf("len: %d\t", len);
//        for (int i = 7; i >=0; --i) {
//            printf("%d", getBit(byte, i));
//        }
        printf("\n");

        fread(&byte, 1, 1, f);
        unsigned char symbol = byte;
        printf("symbol: %c\n", byte);
//        for (int i = 7; i >=0; --i) {
//            printf("%d", getBit(byte, i));
//        }

        fread(&byte, 1, 1, f);
        printf("code: ");
        char *code = (char *) (calloc(9, sizeof(char)));
        for (int i = len - 1, j = 0; i >= 0; --i, ++j) {
            code[i] = getBit(byte, i) == 1 ? '1' : '0';
        }
        printf("%s", code);
        huffmanCode[(int) (symbol)] = code;
        printf("\n\n");
    }

    node *root = createTreeFromCodes(huffmanCode);

    // Read rest of file
    unsigned char offset_byte;
    fread(&offset_byte, 1, 1, f);
    printf("offset = %d\n", binaryToDecimal(offset_byte));

    char *encoded = (char *) (calloc(1000000, 1));
    unsigned char sym;
    int i = 0;
    while (!feof(f)) {
        fread(&sym, 1, 1, f);
        for (int j = 0; j < 8; ++j) {
            encoded[i++] = getBit(sym, j) == 0 ? '0' : '1';
        }
    }
    encoded[strlen(encoded) - binaryToDecimal(offset_byte) - 8] = '\0';
    int index = -1;
    printf("\nDecoded string is :\n");
//    for (int i = 0; i < strlen(encoded) / 2; ++i) {
//        char tmp = encoded[i];
//        encoded[i] = encoded[strlen(encoded) - 1 - i];
//        encoded[strlen(encoded) - 1 - i] = tmp;
//    }
FILE* res = fopen("decoded.txt", "w");
    while (index < (int) (strlen(encoded)) - 1) {
        decode(root, &index, encoded, res);
    }
    fclose(res);
    free(encoded);

    for (int i = 0; i < alphabet_size; ++i) {
        if (huffmanCode[i] != NULL) {
            free(huffmanCode[i]);
        }
    }

    free(huffmanCode);
    fclose(f);
}

/*!
 * Агрегирующая функция, когда программа запускается с флагом `-d filname.zip`.
 *
 * @param filepath[in]: Путь к файлу со сжатым текстом.
 */
void decompressHuffman(const char *filepath) {
    parseFile(filepath);
}
