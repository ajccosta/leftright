#include <iostream>
//#include <mpm/2writer_leftright.h>
#include <mpm/leftright.h>
#include <atomic>
#include <map>
#include <vector>
#include <chrono>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_READER_REGISTERS 4

//Avoid having to type the name of the map every time
using lrkey = int;
using lrval = int;
//using lrmap = mpm::basic_leftright<std::map<lrkey, lrval>, mpm::distributed_atomic_reader_registry<NUM_READER_REGISTERS>>;
using lrmap = mpm::leftright<std::map<lrkey, lrval>>;


void
write(lrmap &lrm, lrkey key, lrval val)
{
    lrm.modify([key, val](lrmap::reference map) noexcept {
        map[key] = val;
    });
}


lrval
read(lrmap &lrm, lrkey key)
{
    int value = lrm.observe([key](lrmap::const_reference map) {
        return map.find(key)->second;
    });
    return value;
}


int
main(int argc, char * argv[])
{
    int opt;

    int num_readers = 10;
    int num_writers = 4;
    bool silent = false;
    std::chrono::duration<double> time = std::chrono::duration<double>(5); //number of seconds to run experiment for

    while((opt = getopt(argc, argv, "w:r:st:")) != -1) {
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
	  		default:
				break;
		}
	}

    lrmap* lrm = new lrmap;


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
        threads.push_back(std::thread([lrm, finish_ref, num_write_ops_ref]{
            while(!*finish_ref)
            {
                uint32_t num_write_ops_t = 0;
                int key = 0;
                int val = 10;
                while(!*finish_ref)
                {
                    write(*lrm, key, val);
                    num_write_ops_t++;
                }
                (*num_write_ops_ref).fetch_add(num_write_ops_t);
            }
        }));
    }

    for(int i = 0; i < num_readers; i++)
    { //Writers
        threads.push_back(std::thread([lrm, finish_ref, num_read_ops_ref]{
            uint32_t num_read_ops_t = 0;
            int key = 0;
            while(!*finish_ref)
            { 
                read(*lrm, key);
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


    std::cout << "Number of read operations: " << num_read_ops << '\n'
        << "Number of write operations: " << num_write_ops << '\n'
        << "Total number of operations: " << num_read_ops + num_write_ops << '\n'
        << "Throughput: " << double (num_read_ops + num_write_ops) / elapsed_seconds.count() << " op / s" << '\n'
        << "Time elapsed: " << elapsed_seconds.count() << '\n';

    return 0;
}
