#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <random>


std::condition_variable condition_variable;
std::mutex cout_guard;

void sort_from_l_to_r(double* arr, int l, int r, const char* thread_name, bool flag) {
	for (int i = l-1; i < r; ++i){
		int min = i;
		for (int j = i + 1; j < r; j++) {
			if (arr[min] > arr[j]) {
				min = j;
			}
			if (flag) 		std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
		std::swap(arr[i], arr[min]);
	}

	std::cout << std::endl;
	for (int i = l - 1; i < r; ++i) {
		const std::lock_guard<std::mutex> lock(cout_guard);
		std::cout << std::endl << thread_name << ' ';
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		std::cout << arr[i];
		if (!flag) 	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	condition_variable.notify_one();
}

const int ARR_SIZE = 20;

int main() {
	const char* thread_name1 = "thread1";
	const char* thread_name2 = "thread2";
	const char* thread_name3 = "thread3";
	srand(time(NULL));
	double array[ARR_SIZE];
	for (int i = 0; i < ARR_SIZE; i++) {
		array[i] = static_cast<double>(1 + rand() % 2002) / 100.0;
	}

	std::pair<int, int> first_half = std::make_pair(1, ARR_SIZE / 2);
	std::pair<int, int> second_half = std::make_pair(ARR_SIZE / 2 + 1, ARR_SIZE);

	// Sorting the first half of array
	std::thread thread1(sort_from_l_to_r, array, first_half.first, first_half.second, thread_name1, true);
	// Sorting the second half of array
	std::thread thread2(sort_from_l_to_r, array, second_half.first, second_half.second, thread_name2, false);

	{
		std::unique_lock<std::mutex> lock(cout_guard);
		condition_variable.wait_for(lock, std::chrono::duration<double, std::milli>(1e9));
		condition_variable.wait_for(lock, std::chrono::duration<double, std::milli>(1e9));
	}
	std::cout << std::endl << "The array after 1,2 threads " << std::endl;
	for (int i = 0; i < ARR_SIZE; ++i) {
		std::cout << array[i] << ' ';
	}
	// Sorting the full array
	std::thread thread3(sort_from_l_to_r, array, first_half.first, second_half.second, thread_name3, false);

	{
		std::unique_lock<std::mutex> lock(cout_guard);
		condition_variable.wait_for(lock, std::chrono::duration<double, std::milli>(1e9));
		std::cout << std::endl << "The threads have finished working";
	}

	thread1.join();
	thread2.join();
	thread3.join();

	std::cout << std::endl << "The array in main thread is " << std::endl;
	for (int i = 0; i < ARR_SIZE; ++i) {
		std::cout << array[i] << ' ';
	}

	std::cout << std::endl << "Main thread has stopped working" << std::endl;
	return 0;
}