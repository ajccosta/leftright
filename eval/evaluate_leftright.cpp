#include <iostream>
#include <atomic>
#include <vector>
#include <chrono>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <mpm/twowriter_leftright.h>
#include <mpm/leftright.h>

#include "utils/wyhash.h"

#include "data-structures/map.h"
//#include "data-structures/BSTNode.h"


#define NUM_READER_REGISTERS 4

//Avoid having to type the name of the map every time
//using lri = mpm::basic_leftright<TYPE, mpm::distributed_atomic_reader_registry<NUM_READER_REGISTERS>>;
using lri = mpm::leftright<TYPE>;

//using lri = twl::twowriter_leftright<TYPE, twl::distributed_atomic_reader_registry<NUM_READER_REGISTERS>>;
//using lri = twl::leftright<TYPE>;


template <typename T, typename K, typename V>
void
write(T *d, K key, V val)
{
    d->modify([key, val](T::reference d) noexcept {
        WRITE(d, key, val);
        //std::cout << "Inserted " << WRITE(d, key, val) << std::endl;
    });
}


template <typename T, typename K>
void
read(T *d, K key)
{
    d->observe([key](T::const_reference d) noexcept {
        READ(d, key);
        //std::cout << "Found " << READ(d, key) << std::endl;
    });
}

int
main(int argc, char * argv[])
{
    int opt;

    int num_readers = 10;
    int num_writers = 4;
    bool silent = false;
    std::chrono::duration<double> time = std::chrono::duration<double>(5); //number of seconds to run experiment for
    int keyspace_size = 10000;

    while((opt = getopt(argc, argv, "w:r:st:k:")) != -1) {
		switch(opt) {
			case 'w':
				num_writers = atoi(optarg);
				break;
			case 'r':
				num_readers = atoi(optarg);
				break;
			case 's':
				silent = true;
				break;
			case 't':
                time = std::chrono::duration<double>(atof(optarg));
				break;
			case 'k':
                keyspace_size = atoi(optarg);
				break;
	  		default:
				break;
		}
	}

    lri* lrm = new lri;


    std::vector<std::thread> threads;
    bool finish = false;
    bool *finish_ref = &finish;

    std::atomic_uint_fast32_t num_read_ops {0};
    std::atomic_uint_fast32_t* num_read_ops_ref = &num_read_ops;

    std::atomic_uint_fast32_t num_write_ops {0};
    std::atomic_uint_fast32_t* num_write_ops_ref = &num_write_ops;

    const auto start{std::chrono::steady_clock::now()};

    for(int i = 0; i < num_writers; i++)
    { //Readers
        threads.push_back(std::thread([&lrm, finish_ref, num_write_ops_ref, i, keyspace_size]{
            while(!*finish_ref)
            {
                uint32_t num_write_ops_t = 0;
                uint64_t seed = i;

                while(!*finish_ref)
                {
                    int key = HASH(&seed) % keyspace_size;
                    int val = HASH(&seed);
                    write(lrm, key, val);
                    num_write_ops_t++;
                }
                (*num_write_ops_ref).fetch_add(num_write_ops_t);
            }
        }));
    }

    for(int i = 0; i < num_readers; i++)
    { //Writers
        threads.push_back(std::thread([&lrm, finish_ref, num_read_ops_ref, i, keyspace_size]{
            uint32_t num_read_ops_t = 0;
            uint64_t seed = i;

            while(!*finish_ref)
            { 
                int key = HASH(&seed) % keyspace_size;
                read(lrm, key);
                num_read_ops_t++;
            }
            (*num_read_ops_ref).fetch_add(num_read_ops_t);
        }));
    }

    std::chrono::duration<double> elapsed_seconds;
    auto end{std::chrono::steady_clock::now()};

    while((elapsed_seconds = end - start) < time)
    {
        std::this_thread::yield();
        end = std::chrono::steady_clock::now();
    }

    finish = true;
    for(auto &t: threads) { t.join(); }


    if(!silent)
    {
        std::cout << std::fixed << "Number of read operations: " << num_read_ops << '\n'
            << "Number of write operations: " << num_write_ops << '\n'
            << "Total number of operations: " << num_read_ops + num_write_ops << '\n'
            << "Throughput: " << double (num_read_ops + num_write_ops) / elapsed_seconds.count() << " op / s" << '\n'
            << "Time elapsed: " << elapsed_seconds.count() << '\n';
    }
    else
    {
        std::cout << std::fixed << double (num_read_ops + num_write_ops) / elapsed_seconds.count() << '\n';
    }

    return 0;
}
