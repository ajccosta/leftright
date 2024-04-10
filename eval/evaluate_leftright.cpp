#include <iostream>
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
using lrmap = mpm::basic_leftright<std::map<lrkey, lrval>, mpm::distributed_atomic_reader_registry<NUM_READER_REGISTERS>>;
//using lrmap = mpm::leftright<std::map<lrkey, lrval>>;


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
    int num_read_ops = 1000000;
    int num_writers = 4;
    int num_write_ops = 10000;

    while((opt = getopt(argc, argv, "w:r:l:k:")) != -1) {
		switch(opt) {
			case 'w':
				num_writers = atoi(optarg);
				break;
			case 'r':
				num_readers = atoi(optarg);
				break;
			case 'l':
				num_write_ops = atoi(optarg);
				break;
			case 'k':
				num_read_ops = atoi(optarg);
				break;
	  		default:
				break;
		}
	}

    lrmap* lrm = new lrmap;


    std::vector<std::thread> threads;

    num_read_ops = num_readers > 0 ? num_read_ops / num_readers : 0;
    num_write_ops = num_writers > 0 ? num_write_ops / num_writers : 0;

    const auto start{std::chrono::steady_clock::now()};
    for(int i = 0; i < num_readers; i++)
    { //Writers
        threads.push_back(std::thread([lrm, num_read_ops]{
            int key = 0;
            for(int i = 0; i < num_read_ops; i++)
            { 
                read(*lrm, key);
            }
        }));
    }

    for(int i = 0; i < num_writers; i++)
    { //Readers
        threads.push_back(std::thread([lrm, num_write_ops]{
            int key = 0;
            int val = 10;
            for(int i = 0; i < num_write_ops; i++)
            {
                write(*lrm, key, val);
            }
        }));
    }

    for(auto &t: threads)
    {
        t.join();
    }
    const auto end{std::chrono::steady_clock::now()};

    const std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << elapsed_seconds.count() << '\n';

    return 0;
}
