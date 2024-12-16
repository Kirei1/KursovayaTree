
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 10
#define MAX_LINE_LENGTH 256

typedef struct Person {
    int id;
    char name[50];
    char surname[50];
    int age;
    char birthDate[12];
    struct Person* parent;
    struct Person* children[MAX_CHILDREN];
    int childCount;
} Person;

// Функция создания нового узла
Person* createPerson(int id, const char* name, const char* surname, int age, const char* birthDate) {
    Person* newPerson = (Person*)malloc(sizeof(Person));
    if (!newPerson) {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }
    newPerson->id = id;
    strcpy(newPerson->name, name);
    strcpy(newPerson->surname, surname);
    newPerson->age = age;
    strcpy(newPerson->birthDate, birthDate);
    newPerson->parent = NULL;
    newPerson->childCount = 0;
    return newPerson;
}

// Поиск узла по ID
Person* findPersonByID(Person** allPersons, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (allPersons[i]->id == id) {
            return allPersons[i];
        }
    }
    return NULL;
}

// Поиск узла по имени
Person* findPersonByName(Person* root, const char* name) {
    if (!root) return NULL;
    if (strcmp(root->name, name) == 0) return root;
    for (int i = 0; i < root->childCount; i++) {
        Person* found = findPersonByName(root->children[i], name);
        if (found) return found;
    }
    return NULL;
}

// Получение всех потомков
void getDescendants(Person* person, int level) {
    if (!person) return;
    for (int i = 0; i < level; i++) printf("  ");
    printf("%s %s (возраст: %d)\n", person->name, person->surname, person->age);
    for (int i = 0; i < person->childCount; i++) {
        getDescendants(person->children[i], level + 1);
    }
}

// Определение ближайшего общего предка
Person* findCommonAncestor(Person* p1, Person* p2) {
    if (!p1 || !p2) return NULL;
    while (p1) {
        Person* temp = p2;
        while (temp) {
            if (temp == p1) return temp;
            temp = temp->parent;
        }
        p1 = p1->parent;
    }
    return NULL;
}

// Связывание родителей и детей
void linkParentChild(Person* parent, Person* child) {
    if (parent->childCount < MAX_CHILDREN) {
        parent->children[parent->childCount++] = child;
        child->parent = parent;
    } else {
        printf("У %s слишком много детей\n", parent->name);
    }
}

// Построение дерева из CSV
Person* buildTreeFromCSV(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Ошибка открытия файла %s\n", filename);
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    fgets(line, MAX_LINE_LENGTH, file); // Пропускаем заголовок

    Person* allPersons[MAX_CHILDREN * 10] = {NULL};
    int personCount = 0;

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        int id, parentID, age;
        char name[50], surname[50], birthDate[12];
        sscanf(line, "%d,%49[^,],%49[^,],%d,%11[^,],%d",
               &id, name, surname, &age, birthDate, &parentID);

        // Создаем узел
        Person* person = createPerson(id, name, surname, age, birthDate);
        allPersons[personCount++] = person;

        // Если есть родитель, устанавливаем связь
        if (parentID > 0) {
            Person* parent = findPersonByID(allPersons, personCount, parentID);
            if (parent) {
                linkParentChild(parent, person);
            }
        }
    }

    fclose(file);
    return allPersons[0];
}

// Рекурсивное отображение дерева
void displayTree(Person* root, int level) {
    if (!root) return;
    for (int i = 0; i < level; i++) printf("  ");
    printf("%s %s (возраст: %d, род. %s)\n", root->name, root->surname, root->age, root->birthDate);
    for (int i = 0; i < root->childCount; i++) {
        displayTree(root->children[i], level + 1);
    }
}

void generateGraphviz(Person* root, FILE* file) {
    if (!root) return;
    for (int i = 0; i < root->childCount; i++) {
        fprintf(file, "\"%s %s\" -> \"%s %s\";\n", root->name, root->surname, root->children[i]->name, root->children[i]->surname);
        generateGraphviz(root->children[i], file);
    }
}

void deleteTree(Person* root) {
    if (!root) return;
    for (int i = 0; i < root->childCount; i++) {
        deleteTree(root->children[i]);
    }
    free(root);
}

int main() {
    const char* filename = "familydb.csv";

    Person* root = buildTreeFromCSV(filename);

    printf("Генеалогическое древо:\n");
    displayTree(root, 0);

    // Поиск узла
    char searchName[50];
    printf("\nВведите имя для поиска: ");
    scanf("%s", searchName);
    Person* foundPerson = findPersonByName(root, searchName);
    if (foundPerson) {
        printf("Найден: %s %s (возраст: %d, род. %s)\n", foundPerson->name, foundPerson->surname, foundPerson->age, foundPerson->birthDate);
    } else {
        printf("Человек с именем %s не найден.\n", searchName);
    }

    // Визуализация дерева
    FILE* graphvizFile = fopen("tree.dot", "w");
    fprintf(graphvizFile, "digraph FamilyTree {\n");
    generateGraphviz(root, graphvizFile);
    fprintf(graphvizFile, "}\n");
    fclose(graphvizFile);
    printf("Файл tree.dot создан. Используйте Graphviz для визуализации.\n");

    deleteTree(root);

    return 0;
}
