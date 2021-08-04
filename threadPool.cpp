#include "threadpool.h"
#include <iostream>

void fun1(int slp)
{
	cout << "fun1 ..... " << this_thread::get_id() << endl;;
	if (slp > 0)
	{
		cout << " ===== fun1 sleep " << slp << " ==========" << this_thread::get_id();
	}
}

class A
{
public:
	static int AFunc(int n = 0)
	{
		cout << n << " hello Afun! " << this_thread::get_id() << endl;
		return n;
	}

	static string BFunc(int n, string str, char c)
	{
		cout << n << " " << "str == " << str.c_str() << " c == " << c << this_thread::get_id() << endl;
		return str;
	}
};

int main()
{
	try {
		threadpool pool(50);
		A a;
		future<void> f1 = pool.commit(fun1, 0);
		future<int> af = pool.commit(a.AFunc, 9999);
		future<string> bf = pool.commit(A::BFunc, 9987, "freezer", 'a');
		std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::microseconds(900));
		for (int i = 0; i < 50; i++)
		{
			pool.commit(fun1, i * 100);
		}
		std::cout << " =======  commit all ========= " << std::this_thread::get_id() << " idlsize=" << pool.Count() << std::endl;

		std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));
		f1.get();

		cout << "af result" << af.get() << " bf result" << bf.get() << endl;

		std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << " =======  fun1,55 ========= " << std::this_thread::get_id() << std::endl;

		pool.commit(fun1, 55).get();

		std::cout << "end... " << std::this_thread::get_id() << std::endl;

		threadpool pool1(4);
		std::vector< std::future<int> > results;

		for (int i = 0; i < 8; ++i) {
			results.emplace_back(
				pool1.commit([i] {
					std::cout << "hello " << i << std::endl;
					std::this_thread::sleep_for(std::chrono::seconds(1));
					std::cout << "world " << i << std::endl;
					return i * i;
					})
			);
		}
		std::cout << " =======  commit all2 ========= " << std::this_thread::get_id() << std::endl;

		for (auto&& result : results)
			std::cout << result.get() << ' ';
		std::cout << std::endl;
		return 0;
	}
	catch (std::exception& e) {
		std::cout << "some unhappy happened...  " << std::this_thread::get_id() << e.what() << std::endl;
	}
	

}

