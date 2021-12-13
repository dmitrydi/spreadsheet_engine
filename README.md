# Электронная таблица
Проект - движок электронной таблицы, упрощенного аналога MS Excel или Google Spreadsheets (далее - *существующие решения*).
Основные классы движка - класс собственно таблицы `ISheet` и класс ячеек `ICell`. Поддерживает вычисление формул. Для парсинга формул используется генератор парсеров [ANTLR 4](https://www.antlr.org/).

# Установка
### Требования
Установленная Java 7.

### Сборка (для Windows/Eclipse IDE)
Сборка осуществляется с помощью cmake  

1. Создать на одном уровне с папкой `src` папку `build`
2. Перейти в папку `build` и выполнить команду `cmake -G "Eclipse CDT-4 MinGW Makefiles" ..\src`
3. В Eclipse IDE сделать импорт собранного проекта из папки build

# Использование

Таблица состоит из ячеек. Интерфейс таблицы позволяет создавать ячейки с текстои или формулами, а так же распечатывать содержимое ячеек в виде текста или значений вычисленных формул.  

### Создание пустой таблицы

	std::unique_ptr<ISheet> sheet = CreateSheet();

Создает умный указатель на пустую таблицу.  
Заполнение ячеек осуществляется с помощью метода `SetCell` (см [далее](#setcell)). 


### Доступ к ячейкам

Доступ к ячекам таблицы осуществляется с помощью методов:  

	ICell* GetCell(Position pos);
	const ICell* GetCell(Position pos) const;

В случае, если ячейка с позицией pos не была заполнена или была удалена, то возвращается `nullptr`:  

	sheet->SetCell(Postion::FromString("A1"), "hello");
	assert(sheet->GetCell(Position::FromString("A1")));
	assert(!sheet->GetCell(Position::FromString("A2")));

### Заполнение ячеек таблицы<a name = setcell></a>

Заполнение ячеек таблицы осуществляется с помощью метода `ISheet::SetCell(Position pos, std::string text)`, где `Position`  - структура с полями `{ int row, int column}`.  
Для задание позиции ячеки в формате, аналогичном *существующим решениям*, используется функция `Position::FromString(std::string)`:  

	auto sheet = CreateSheet();
	sheet->SetCell({0,0}, "Hello, world");
	sheet->SetCell(Position::FromString("B1"), "Another text");

Позиция ячейки должна быть не меньше, чем `{0, 0}` и не больше, чем `{16383, 16383}` (от "A1" до "XFD16384").  
В отличие от *существующих решений*, в данной реализации ячейка может содержать либо текст, либо число с плавающей точкой, причем последнее может возникнуть только как результат вычисления формулы.  

#### Формулы
Ячейка трактуется как формула в случае, если её текст начинается со знака "=", после которого следуют другие символы (далее - *выражение формулы*). Если перед знаком "=" стоит символ "'", то содержимое трактуется как текст.  
*Выражение формулы* - простое арифметическое выражение, которое может соержать:  

+ Числа  
+ Бинарные операции  
+ Ссылки на другие ячейки  

Если формула ссылается на другую ячейку, которая не содержит формулу, то ее текст трактуется как число. Если этот текст полность нельзя трактовать как число (например, "11AM"), то
при попытке вычислить такую формулу возвращается значение типа `FormulaError`:  

	auto sheet = CreateSheet();
	sheet->SetCell(Position::FromString("A1"), "=1");
	sheet->SetCell(Position::FromString("A2"), "=A1 + 1");
	sheet->SetCell(Position::FromString("A3"), "=A2 + B1");
	std::cout << PrintValue(sheet->GetCell(Position::FromString("A3")->GetValue())); // 2

	sheet->SetCell(Position::FromString("B1"), "text");
	std::cout << PrintValue(sheet->GetCell(Position::FromString("A3")->GetValue())); // "#VALUE!"

При попытке записать в ячейку синтаксически некорректную формулу, например, "=A1+", выбрасывается исключение FormulaException, при этом содержимое ячеки, существовавшее до указанной попытки, не меняется.  
При попытке сослаться в формуле на ячейку с некорректной позицией, например, {-1,-1} или {16384, 16384} выбрасывается исключение InvalidPositionException, при этом содержимое ячеки, существовавшее до указанной попытки, не меняется.  
Если производится попытка записать в ячейку формулу, которая привела бы к циклической зависимости между ячейками, то выбрасывается исключение CircularDependencyException,  при этом содержимое ячеки, существовавшее до указанной попытки, не меняется:  

	auto sheet = CreateSheet();
	sheet->SetCell(Position::FromString("A1"), "1");
	sheet->SetCell(Position::FromAstring("A2"), "=A1");
	bool caught = false;

	try {
		sheet->SetCell(Position::FromString("A1"), "=A2");
	} catch (CircularDependencyException& ex) {
		caught = true;
	}

	assert(caught);
	assert(sheet->GetCell(Position::FromString("A1"))->GetText() == "1");

#### Получение содержимого ячеек
Интерфейс таблицы предоставляет следующие методы получения содержимого ячеек:

1. `std::string ICell::GetText()` - возвращает внутренний текст ячейки. В случае текстовой ячеки - это текст, содержащий, возможно, экранирующие символы, в случае формулы - её *выражение*.
2. `std::string ICell::GetValue()` - возвращает значение ячейки. В случае текстовой ячейки - это ее внутренний текст без экранирующих символов, в случае формулы - объект типа `std::variant<double, FormulaError>`.

Примеры:  

	auto sheet = CreateSheet();
	sheet->SetCell(Position::FromString("A1"), "'=1");
	assert(sheet->GetCell(Position::FromString("A1"))->GetText() == "'=1"); 			// экранирующий символ перед знаком "="
	assert(sheet->GetCell(Position::FromString("A1"))->GetValue() == "=1"); 			// экранирующий символ убран

	sheet->SetCell(Position::FromString("A2"), "=(B1 + B2) + 1");
	assert(sheet->GetCell(Position::FromString("A1"))->GetText() == "=B1 + B2 + 1"); 	// выражение формулы без лишних скобок
	assert(sheet->GetCell(Position::FromString("A1"))->GetValue() == ICell::Value{1}); 	// вычисленное значение формулы

	sheet->SetCell(Position::FromString("A2"), "=A1 + A2");
	assert(sheet->GetCell(Position::FromString("A1"))->GetText() == "=A1 + A2");		// текст корректной формулы парсится даже при невозможности вычислить значение
	assert(std::get<FormulaError>(sheet->GetCell(Position::FromString("A1"))->GetValue()).GetCategory() == FormulaError::Category::Value) // в ячейке "A1" записан текст, а не формула

### Очистка ячейки

Очистка ячейки производится с помощью функции `ICell::ClearCell(Position pos)`. Повторное обращение к ячейке вернет `nullptr`.

### Вставка и удаление строк или столбцов

Вставка строк и столбцов производится с помощью функций `ISheet::InsertRows(int before, int count = 1)` и `ISheet::InsertCols(int before, int count = 1)`  
При вставке строк/столбцов *выражения* ячеек меняются консистентным образом, при этом зачения формул в ячейках не меняются:  

	auto sheet = CreateSheet();
	sheet->SetCell(Position::FromString("A1"), "=A2");
	sheet->SetCell(Position::FromString("A2"), "=1");
	sheet->InsertRows(1);
	assert(sheet->GetCell(Position::FromString("A1"))->GetText() == "=A3");
	assert(sheet->GetCell(Position::FromString("A1"))->GetValue() == ICell::Value{1});

Если при поптыке вставить столбцы/строки какие-либо ячейки или ссылки на ячейки выйдут за границы допустимой области, будет выброшено исключение TableTooBigException, при этом содержимое таблицы не изменится:  

	auto sheet = CreateSheet();
	sheet->SetCell(Position::FromString("A1"), "=XFD16384");
	bool caught = false;
	try {
		sheet->InsertRows(1);
	} catch (TableTooBigException&) {
		caught = true;
	}
	assert(caught);
	assert(sheet->GetCell(Position::FromString("A1"))->GetText() == "=XFD16384"); 
	

Удаление происходит при помощи функций `ISheet::DeleteRows(int first, int count = 1)` и `ISheet::DeleteCols(int first, int count = 1)` .
Если будет удалена строка или столбец с ячейками, на которые ссылаются другие ячейки, то в *выражениях* зависимых ячеек появится указание на неверную ссылку, а сами ячейки примут ошибочное состояние FormulaError:  

	auto sheet = CreateSheet();
	sheet->SetCell(Position::FromString("A2"), "=A1");
	sheet->SetCell(Position::FromString("A3"), "=A2");
	sheet->DeleteRows(0);
	assert(sheet->GetCell(Position::FromString("A1"))->GetText() == "#REF!");
	assert(sheet->GetCell(Position::FromString("A2"))->GetText() == "=A1");
	assert(std::get<FormulaError>(sheet->GetCell(Position::FromString("A1"))->GetValue()).GetCategory() == FormulaError::Category::Ref);
	assert(std::get<FormulaError>(sheet->GetCell(Position::FromString("A2"))->GetValue()).GetCategory() == FormulaError::Category::Ref)

### Получение размеров таблицы

Получение размера области печати производится с помощью функции `ISheet::GetPrintableSize()`. Область печати - это минимальный прямоугольник с левой верхней вершиной в ячейке А1, охватывающий все ячейки с непустым текстом:  

	auto sheet = CreateSheet();
	auto sz = sheet->GetPrintableSize();
	assert(sz.rows == 0);
	assert(sz.cols == 0);
	sheet->SetCell(Position::FromString("B2"), "=C3");
	sz = sheet->GetPrintableSize();
	assert(sz.rows == 2);
	assert(sz.cols == 2);

### Печать таблицы

С помощью методов `ISheet::PrintValues(std::ostream&)` и `ISheet::PrintTexts(std::ostream&)` таблицу можно распечатать целиком в некоторый поток. При этом будет выведена минимальная прямоугольная область, включающая в себя все непустые ячейки:  

	auto sheet = CreateSheet();
    sheet->SetCell(Position::FromString("A2"), "text");
    sheet->SetCell(Position::FromString("B2"), "=35");

    std::ostringstream texts;
    sheet->PrintTexts(texts);
    assert(texts.str() == "\t\ntext\t=35\n");

    std::ostringstream values;
    sheet->PrintValues(values);
    assert(values.str() == "\t\ntext\t35\n");


