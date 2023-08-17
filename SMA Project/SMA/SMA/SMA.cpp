
// warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#define _CRT_SECURE_NO_WARNINGS

// Используем библиотеки stl
#include <iostream>
#include <chrono>

// включаем пространство имен
using namespace std;

// предопределённые окна
#define SMAW_4		4
#define SMAW_8		8
#define SMAW_16		16
#define SMAW_32		32
#define SMAW_64		64
#define SMAW_128	128

// определяем тип - указатель на double как LPDOUBLE
typedef double * LPDOUBLE;

// Функция Simple moving average
// bool puresignal			- особый режим функции (плавная)
// LPDOUBLE& calculation	- выходной массив (выделять память не нужно)
// double* indication		- входной массив
// unsigned long width		- размер окна
// unsigned long size		- размер входного массива
// bool& error				- индикатор ошибки

double SMA(bool puresignal,LPDOUBLE &calculation, double *indication, unsigned long width, unsigned long size, bool &error) {
	if(size <= 0){ // если размер входных данных меньше или равен нулю то, возвращаем с ошибкой
		error = false;
		return 1.0;
	}

	if((width <= 0) || (width > SMAW_128)){ // если размер окна меньше или ровно нулю или размер окна более SMAW_128, возвращаем с ошибкой
		error = false;
		return 2.0;
	}

	unsigned long corr_size = (unsigned long)(round((double)size/(double)width)*(double)width); // корректируем размер входномо массива
	calculation = (double*)malloc(sizeof(double)*(corr_size)); // выделяем память в куче под выходной массив данных
	memset(calculation,0,sizeof(double)*(corr_size)); // задаём на выходной массив нули

	// определяем нынешнее время since epoc в наносекундах
	uint64_t time_old = chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	double old_mm = indication[0]; // базовое значение - как старое
	unsigned long c_count = 0; // счётчик элементов для calculation
	for (unsigned long i = 0; i < corr_size; i += width) { // шаг с размером окна 0 - 4, 4 - 8 итд...
		// среднее арифметическое (в промежутке)
		double mm = 0.0;
		for (unsigned long j = 0; j < width; j++) {
			mm += indication[i + j];
		} mm /= (double)width;

		if(puresignal){ // Усложнение задачи (!), для формирофания более чистой выходной синусоиды (Может быть полезно при больших промежутках), ВНИМАНИЕ! контролируется входным параметром puresignal
			double dstep = (mm-old_mm)/(double)width; // разница между предыдущим и нынешнем значениями деленное на размер окна
			for (unsigned long j = 0; j < width; j++) {
				calculation[c_count] = old_mm; // заполняем выходной массив откорректированными значениями
				old_mm += dstep; // плавное перетекание от предыдущего к нынешнему значению mm
				c_count++;
			} old_mm = mm; // сохранение предыдущего значения
		}else{
			for (unsigned long j = 0; j < width; j++) { // заполняем выходной массив одинаковыми данными для сохранения размера и соответствия
				calculation[c_count] = mm;
				c_count++;
			}
		}
	}

	// вычисляем потраченое время в наносекуднах на алгоритм работы SMA функции
	uint64_t t2 = chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - time_old;
	return (double)(t2)/1000000000.0; // возвращаем время в секундах затраченое на выполнение алгоритма SMA
}

int main(int argc,char **argv){
	setlocale(LC_NUMERIC,""); // см. стр. 91,115
	srand(time(NULL)); // сброс рандомайзера

	unsigned long s_size = 1000000; // размер входного массива
	bool puresignal = true; // см. стр. 58

	// (ВЫВОД)
	FILE *noise_sinef = fopen("noise_sine.txt","w"); // создаем перезаписываемый файл для значений шумной синусоиды
	double step = 0.0; // переменная - шаг синусоиды
	double *noise_sine = (double*)malloc(sizeof(double)*s_size); // выделяем память в куче под входной массив с шумной синусоидой
	for(unsigned long i = 0;i<s_size;i++){
		noise_sine[i] = sin(step)+(double)(rand()%s_size)*0.000021; // синус + рандомный шум
		fprintf(noise_sinef,"%f\n",noise_sine[i]); // записываем значения в файл (запись чисел с плавющей точной будет производится не с точной а запятой см. стр. 79)
		step += 0.01; // шаг синусоизы 0.01
	} fclose(noise_sinef); //закрываем файл


	unsigned int modes[6] = {SMAW_4,SMAW_8,SMAW_16,SMAW_32,SMAW_64,SMAW_128};
	bool err[6] = {false,false,false,false,false,false}; // переменная ошибки
	double* calculations[6] = {0,0,0,0,0,0}; // массив выходных данным
	double t[6] = {0.0,0.0,0.0,0.0,0.0,0.0}; // время выполнения
	double t_dt = 0.0; // общее время выполнения для всех режимов

	// РАБОТА
	for(unsigned char i = 0;i<6;i++){
		t[i] = SMA(puresignal, calculations[i], noise_sine, modes[i], s_size, err[i]); // выполнение алгоритма SMA и подсчёт затраченого времени в секундах
		if (err[i]) { // если ошибка, то выводим сообщение об ошибке с кодом далее пауза и выход
			cout << "SMA_" << modes[i] << " error: " << t[i] << endl;
			system("PAUSE");
			return 0;
		}

		// ВЫВОД
		char path[256]; sprintf(path,"SMA_%d_calcs.txt",modes[i]);
		FILE* SMA_calcs = fopen(path, "w"); // создаем перезаписываемый файл для выходных значений
		for (unsigned long j = 0; j < s_size; j++) {
			fprintf(SMA_calcs, "%f\n", calculations[i][j]); // записываем значения в файл (запись чисел с плавющей точной будет производится не с точной а запятой см. стр. 79)
		} fclose(SMA_calcs); //закрываем файл
		free(calculations[i]); // освобождаем память выходного массива

		// вывод затраченого времени м выход из программы
		printf("Performance_%d: %fs\n", modes[i],t[i]);
		t_dt += t[i]; // суммируем время выполнения алгоритма
	} free(noise_sine); // освобождаем память входного массива
	printf("Performance for all: %f\n",t_dt);

	system("PAUSE");
	return 0;
}