//visit the following url before making library
//https://www.codeproject.com/articles/48575/how-to-define-a-template-class-in-a-h-file-and-imp


// asAlloc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<cstdint>
#include<iostream>
#include<cassert>
#include<type_traits>


namespace asAlloc  
{
	namespace core
	{
		//Partiallialy constructed PageManager singleton for general purpose allocators
		//allocating up front 256 MiB of memory from heap
		class PageManager 
		{
		private:
			std::uint8_t* buffer;
			static PageManager* instance;
			static bool instanceFlag;
			static size_t size;

			size_t marker;

			PageManager()
			{
				marker = 0;
				buffer = new std::uint8_t[size];
			}

		public:
			static PageManager* getInstance();

			void doSomething()
			{
				std::cout << std::endl << "did" << std::endl;
			}

			~PageManager()
			{
				instanceFlag = false;
				delete[] buffer;
			}

		};


		size_t PageManager::size = 268435456; //256 MiB
		PageManager* PageManager::instance = nullptr;
		bool PageManager::instanceFlag = false;

		PageManager* PageManager::getInstance()
		{
			if (!instanceFlag)
			{
				instanceFlag = true;
				return new PageManager();
			}
			else
			{
				return instance;
			}
		}


	}


	//specialized allocators
	//1) StackAllocator
	//2) PoolAllocator
	//currently allocate memory from heap, but will later allocate memory from PageManager class

	class  StackAllocator
	{
	private:
		std::uint8_t* buffer;
		size_t sizeOfBuffer;
		void* currTop;
		void* tmp;
		size_t currMarker;

		void* allocateUnAligned(size_t size);

	public:
		typedef size_t Marker;
		explicit StackAllocator(size_t size);

		template<typename T>
		T* allocate(size_t numOfElements=1);
		Marker getCurrentMarker();
		void freeToMarker(Marker marker);
		void clear();
		~StackAllocator()
		{
			delete[] buffer;
		}
	};

	void* StackAllocator::allocateUnAligned(size_t size)
	{
		if ((currMarker + size) >= sizeOfBuffer)
		{
			std::cerr << std::endl << "----------------Errror---------------------" << std::endl << "Memory Full" << std::endl << std::endl;
			return nullptr;
		}

		tmp = currTop;
		currMarker += size;
		currTop = buffer + currMarker;
		return tmp;
	}

	StackAllocator::StackAllocator(size_t size)
	{
		sizeOfBuffer = size;
		buffer = new std::uint8_t[size];
		std::cout << static_cast<void*>(buffer)<<std::endl;
		currTop = buffer;
		currMarker = 0;
	}

	template<typename T>
	T* StackAllocator::allocate(size_t numOfElements)
	{
		size_t size = sizeof(T);
		//size_t alignment = __alignof(T);
		size_t alignment = std::alignment_of<T>::value;

		assert(numOfElements > 0);
		assert(alignment >= 1);
		assert(alignment <= 128);
		assert((alignment&(alignment - 1)) == 0);


		size_t totAlloc = (size*numOfElements) + alignment;
		
		void* rawMemBlock = allocateUnAligned(totAlloc);
		if (rawMemBlock == nullptr)
		{
			return nullptr;
		}
		uintptr_t rawAddress = reinterpret_cast<uintptr_t>(rawMemBlock);

		size_t mask = alignment - 1;
		uintptr_t misaligment = rawAddress & mask;
		ptrdiff_t adjustment = alignment - misaligment;

		uintptr_t alignedAddress = rawAddress + adjustment;
		
		return reinterpret_cast<T*>(alignedAddress);
		
	}
	
	StackAllocator::Marker StackAllocator::getCurrentMarker()
	{
		
		return currMarker;
	}

	void StackAllocator::freeToMarker(StackAllocator::Marker marker)
	{
		currMarker = marker;
		currTop = buffer + currMarker;
	}

	void StackAllocator::clear()
	{
		currMarker = 0;
		currTop = buffer + currMarker;
	}

	
	template<typename T>
	class PoolAllocator
	{
	private:
		T* buffer;
		size_t noOfBlocks;
		std::uintptr_t* front;
		std::uintptr_t* back;

	public:
		PoolAllocator(size_t num);

		T* allocate();
		void deAllocate(T*&);

		~PoolAllocator()
		{
			delete[] buffer;
		}

	};

	template<typename T>
	PoolAllocator<T>::PoolAllocator(size_t num)
	{
		buffer = new T[num];

		std::cout << static_cast<void*>(buffer) << std::endl;

		std::uintptr_t* ptr;

		for (size_t i = 0; i < num; i++)
		{
			ptr = reinterpret_cast<std::uintptr_t*>(buffer + i);

			if (i + 1 != num)
			{	
				*ptr = reinterpret_cast<std::uintptr_t>(buffer + i + 1);
			}
			else
			{
				back = ptr;
			}
		}

		front = reinterpret_cast<std::uintptr_t*>(buffer);

		
	}

	template<typename T>
	T* PoolAllocator<T>::allocate()
	{
		if (front == back)
		{
			std::cerr << std::endl << "----------------Errror---------------------" << std::endl << "Memory Full" << std::endl << std::endl;
			return nullptr;
		}

		T* newBlock = reinterpret_cast<T*>(front);
		front = reinterpret_cast<std::uintptr_t*>(*front);
		return newBlock;
	}

	template<typename T>
	void PoolAllocator<T>::deAllocate(T*& block)
	{
		*back = reinterpret_cast<std::uintptr_t>(block);
		back = static_cast<std::uintptr_t*>(block);
		block = nullptr;
	}

}



/*
using namespace std;
int main()
{
	asAlloc::StackAllocator MyAlloc(100);
	double* l=nullptr;
	
	l = MyAlloc.allocate<double>();
	void* t = l;
	cout << t<<endl<<sizeof(double)<<endl<<MyAlloc.getCurrentMarker();
	cin.get();

	asAlloc::core::PageManager* man = asAlloc::core::PageManager::getInstance();
	man->doSomething();

	

	asAlloc::PoolAllocator<double> pool(20);
	double* a = pool.allocate();
	double* b = pool.allocate();
	double* c = pool.allocate();
	double* d = pool.allocate();

	void* t1 = static_cast<void*>(a);
	void* t2 = static_cast<void*>(d);

	cout << endl << t1 << endl << t2;

	cin.get();
	return 0;
}

*/

