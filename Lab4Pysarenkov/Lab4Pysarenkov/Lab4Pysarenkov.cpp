#include <iostream>
#include <Windows.h>
#include <string>

struct Node {
	int val; // >= 0 - дійсне число, -1 - додавання, -2 - множення, -3 - віднімання, -4/-5 - синус/косинус, від -122 до -97 - літери z-a
	Node* left;
	Node* right;
	Node* next;
	Node(int val) {
		this->val = val;
		this->next = nullptr;
		this->left = nullptr;
		this->right = nullptr;
	}
};

Node* StackTop;

void push(Node* n) {
	if (!StackTop)
		StackTop = n;
	else {
		n->next = StackTop;
		StackTop = n;
	}
}

Node* pop() {
	Node* p = StackTop;
	StackTop = StackTop->next;
	return p;
}

void outp(Node* n) {//Вивід дерева в нормальному порядку
	if (!n)
		return;
	else {
		if (n->val == -2 && (n->left->val == -1 || n->left->val == -3)) //якщо лівий множник є сумою або різницею многочленів, то ми його беремо в дужки
		{
			std::cout << "("; //Відкриваємо дужку
		}
		outp(n->left); //Вивід лівої дитини
		if (n->val == -2 && (n->left->val == -1 || n->left->val == -3))
		{
			std::cout << ") "; //Закриваємо дужку
		}
		std::string sval; //Частина виразу, яка виводиться в даній ітерації рекурсії, тобто дійсне число, змінна або операція
		int x = n->val;
		if (x >= 0)
			sval = std::to_string(x); //Дійсне число
		else if (x >= -122 && x <= -97)
			sval += char(x * (-1)); //Змінна
		else {
			switch (x)
			{
			case -1:
				sval = "+";
				break;
			case -2:
				sval = "*";
				break;
			case -3:
				sval = "-";
				break;
			case -4:
				sval = "sin(";
				break;
			case -5:
				sval = "cos(";
				break;
			}
		}
		std::cout << sval << " ";
		if (n->val == -2 && (n->right->val == -1 || n->right->val == -3)) //І аналогічно, якщо правий множник є сумою або різницею многочленів
		{
			std::cout << "(";
		}
		outp(n->right);
		if (n->val == -4 || n->val == -5 || n->val == -2 && (n->right->val == -1 || n->right->val == -3)) {
			std::cout << ") ";
		}
	}
}

Node* treenode(int val, Node* left, Node* right) {//Створення вузла
	Node* n = new Node(val);
	n->left = left;
	n->right = right;
	return n;
}

Node* copytr(Node* p) {//Копія дерева
	if (p)
		return treenode(p->val, copytr(p->left), copytr(p->right));
	else return NULL;
}

Node* diff(Node* n, char cvar) {
	if (n->val == -cvar) {
		return treenode(1, nullptr, nullptr);
	}
	switch (n->val) {
	case -1: //Диференціювання суми: (u + v)' = u' + v'
		return treenode(-1, diff(n->left, cvar), diff(n->right, cvar));
	case -2://Диференціювання добутку: (uv)' = (u' * v) + (v' * u)
		return treenode(-1, treenode(-2, diff(n->left, cvar), copytr(n->right)), treenode(-2, diff(n->right, cvar), copytr(n->left)));
	case -3://Диференціювання різниці: (u - v)' = u' - v'
		return treenode(-3, diff(n->left, cvar), diff(n->right, cvar));
	case -4: //Диференціювання синуса: (sin(x))' = x' * cos(x)
		return treenode(-2, diff(n->right, cvar), treenode(-5, n->left, n->right));
	case -5://Диференціювання косинуса: (cos(x))' = 0 - x' * sin(x)
		return treenode(-3, treenode(0, nullptr, nullptr), treenode(-2, diff(n->right, cvar), treenode(-4, n->left, n->right)));
	default: //Дійсне число: a' = 0
		return treenode(0, nullptr, nullptr);
	}
}

void deltr(Node* n) {//Видалення вузла і його дітей
	if (!n) return;
	deltr(n->left);
	deltr(n->right);
	delete n;
}

Node* simpl(Node* n) {//Спрощення виразу: видалення зайвих нулів при додаванні, одиниць при множенні
	Node* nl, *nr;
	if (!n) return n; //Якщо у попереднього вузла нема дітей і
	if ((n->val > 0) || (n->val < -96 && n->val > -123)) return n; //якщо у вузлі записане дійсне число або змінна, спрощення не відбувається
	nl = n->left = simpl(n->left);
	nr = n->right = simpl(n->right);
	switch (n->val) {
	case -1:
		if (nl->val == 0) { delete nl; delete n; return nr; }//0 + a = a
		if (nr->val == 0) { delete nr; delete n; return nl; }//a + 0 = a
		return n;//Якщо нема нулів в доданках
	case -2:
		if ((nl->val == 0) || (nr->val == 0)) {//0 * a = 0
			n->val = 0;
			n->left = n->right = nullptr;
			deltr(nl);
			deltr(nr);
			return n;
		}
		if (nl->val == 1) { delete nl; delete n; return nr; } //1 * a = a
		if (nr->val == 1) { delete nr; delete n; return nl; } //a * 1 = a
		return n;
	case -3:
		if (nr->val == 0) { delete nr; delete n; return nl; }//a - 0 = a
		return n;
	}
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	std::cout << "Введіть рядок, який буде переведений в expression tree\n";
	//Приклад 1: 2 x * x cos + (еквівалентно 2x + cos x)
	//Приклад 2: 17 x * 3 x cos 3 x * sin - * + 324 -
	//(еквівалентно 17 * x + 3(cos x - sin (3x)) - 324)
	//Приклад 3: x y * 3 y * sin - 23 x cos * +
	//(еквівалентно xy - sin(3y) + 23cos(x))
	std::string row, sval;
	std::getline(std::cin, row);
	int rl = row.length(), i;
	StackTop = nullptr;
	for (i = 0; i < rl; i++) {
		if (row[i] != ' ')//sval - знак операції/змінна/дійсне число, яке буде занесено у вузол. Його зчитування продовжується
			//доти, доки у введеному рядку не буде досягнуто пробілу.
			sval += row[i];
		if (row[i] == ' ' || i == rl - 1)
		{
			int val;
			if (sval == "+")
				val = -1;
			else if (sval == "*")
				val = -2;
			else if (sval == "-")
				val = -3;
			else if (sval == "sin")
				val = -4;
			else if (sval == "cos")
				val = -5;
			else if (sval.length() == 1 && isalpha(sval[0])) //чи є змінною
				val = -1 * sval[0]; //змінна записується у вигляді числа, що помножене на -1 відповідно до таблиці ASCII
			else //в інших випадках вважаємо, що sval є числом
				val = std::stoi(sval);
			if (val < 0 && val > -4) //операції +, -, * потребують двох аргументів, тому у них буде дві "дитини" - ліва і права
			{
				Node *a, *b, *n;
				n = new Node(val);
				a = pop();
				b = pop();
				n->left = b;
				n->right = a;
				push(n);
			}
			else if (val == -5 || val == -4) { //операції sin і cos потребують лише одного аргумента, тому у них буде одна "дитина" - права
				Node *a, *n;
				n = new Node(val);
				a = pop();
				n->right = a;
				push(n);
			}
			else { //змінні і дійсні числа
				Node* n = new Node(val);
				push(n);
			}
			sval = "";
		}
	}
	outp(StackTop);
	std::cout << "\nВведіть змінну, за якою бажаєте продиференціювати вираз: ";
	char cvar;
	std::cin >> cvar;
	Node* Diff;
	Diff = diff(StackTop, cvar);
	Diff = simpl(Diff);
	std::cout << "Продиференційований за змінною " << cvar << " вираз:\n";
	outp(Diff);
	return 0;
}

