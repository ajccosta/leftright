#include <atomic>
#include <cassert>

//TODO: Find out better way to implement this
thread_local uint64_t my_turn;

namespace pl
{
    class PartialLock
    {
        private:
            std::atomic_uint_fast64_t ticket{0};
            std::atomic_uint_fast64_t turn{0};
    
        public: 
            PartialLock() = default;
            ~PartialLock();
    
            enum class LockType
            {
                FULL = 1,
                PARTIAL,
            };
    
    	    LockType
            lock()
            {
                my_turn = ticket.fetch_add(2, std::memory_order_seq_cst);
                uint64_t t = turn.load(std::memory_order_seq_cst);
                while(t != my_turn && (t + 1) != my_turn) {
                    std::this_thread::yield();
                    t = turn.load(std::memory_order_seq_cst);
                }
                return t == my_turn ? LockType::FULL : LockType::PARTIAL;
            } 
    
    	    void
            unlock_full()
            {
                turn.fetch_add(2, std::memory_order_seq_cst);
            } 
    
    	    void
            unlock_partial(LockType lt)
            {
                if(lt == LockType::FULL)
                {
                    turn.fetch_add(1, std::memory_order_seq_cst);
                }
                else
                {
                    turn.fetch_sub(1, std::memory_order_seq_cst);
                }
            }
    
            void
            wait_for_partial()
            {
                while(turn.load(std::memory_order_seq_cst) != my_turn)
                {
                    std::this_thread::yield();
                }
            }

            void
            wait_for_full()
            {
                while(turn.load(std::memory_order_seq_cst) != my_turn)
                {
                    std::this_thread::yield();
                }
            }
    
            bool
            writers_in_flight()
            {
                return ticket.load(std::memory_order_seq_cst) > my_turn + 2;
            }
    
            bool
            am_i_partial()
            {
                return true;
            }
    }; 

}
