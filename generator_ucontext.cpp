#include <iostream>
#include <ucontext.h> // needs glibc >= 2.8 to pass 64-bit pointers
#include <cstring>
#include <tuple>

template<typename T>
class Generator {
public:
    typedef T rtype;
    Generator() : m_value(NULL), m_end(false) {}
    class iterator {
        Generator<T>* m_gen;
       public:
        iterator(Generator<T>* gen) : m_gen(gen) {
        }
        const T& operator*() const {
            return m_gen->value();
        }
        iterator& operator++() {
            getcontext(&m_gen->main_context1); // hopefully not too slow
            if( m_gen->m_value == NULL ) {
                m_gen = NULL;
            } else {
                m_gen->m_value = NULL;
                //std::cout << "saving main2 context, returning to this="<<m_gen<<std::endl;
                swapcontext(&m_gen->main_context2, &m_gen->loop_context);
                //std::cout << "returned to main2 context"<<std::endl;
            }
            return *this;
        }
        const bool operator!=(const iterator& rhs) const {
            return m_gen != rhs.m_gen;
        }
    };
    const T& value() const {
        //std::cout << "value->"<<(void*)m_value<<std::endl;
        return *m_value;
    }
    static void handler(Generator<T>*arg) {
        (*arg)();
    }
    virtual const iterator begin() {
        m_end = false;
        getcontext(&main_context1); // hopefully not too slow
        if( m_end == true ) {
            return iterator(NULL);
        }
        m_end = true;
        getcontext(&loop_context);
        loop_context.uc_link          = &main_context1;
        loop_context.uc_stack.ss_sp   = iterator_stack;
        loop_context.uc_stack.ss_size = sizeof(iterator_stack);
        loop_context.uc_stack.ss_flags = 0;
        //std::cout << "making loop context, this="<<this<<std::endl;
        makecontext(&loop_context, (void (*)(void)) &Generator<T>::handler, 1, this);
        //std::cout << "saving main2 context"<<std::endl;
        swapcontext(&main_context2, &loop_context);
        //std::cout << "returned to main2 context"<<std::endl;
        return iterator(this);
    }
    virtual const iterator end() {
        return iterator(NULL);
    }
    virtual void operator()() {
    }
    void yield(const T& ref) {
        m_value = &ref;
        //std::cout << "yield->"<<(void*)&ref<<std::endl;
        //std::cout << "saving loop context,this="<<this<<",ref="<<(void*)&ref<<std::endl;
        Generator<T>* th = this;
        swapcontext(&loop_context, &main_context2);
        //std::cout << "returned to loop context,this="<<this<<"th="<<th<<",ref="<<(void*)&ref<<std::endl;
    }
private:
    T const* m_value;
    char iterator_stack[SIGSTKSZ];
    ucontext_t main_context1, main_context2, loop_context;
    bool m_end;
};

template<typename T, T INIT>
class Inc : public Generator<int> { public:
void operator()() {
    for( T i(INIT); ; ++i ) {
        //std::cout << "inc_i="<<i<<std::endl;
        this->yield(i);
    }
}};

template<int COUNT, typename T>
class Take : public Generator<typename T::rtype> { public:
void operator()() {
    int i = 0;
    T t;
    for( auto x : t ) {
        //std::cout << "take_x="<<x<<std::endl;
        this->yield(x);
        ++i;
        if( i >= COUNT ) {
            break;
        }
    }
}};

template<typename T, typename T::rtype MOD>
class Mod : public Generator<typename T::rtype> { public:
void operator()() {
    T t;
   // std::cout << "mod="<<(void*)&t<<std::endl;
    typename T::rtype y;
    for( auto x : t ) {
        y = (x % MOD);
        //std::cout << "mod_y="<<y<<std::endl;
        this->yield(y);
    }
}};

template<typename LHS, typename RHS>
class Zip : public Generator<std::tuple<typename LHS::rtype,typename RHS::rtype>> { public:
void operator()() {
    LHS lhs;
    RHS rhs;
    auto lhs_it = lhs.begin();
    auto rhs_it = rhs.begin();
    const auto lhs_end = lhs.end();
    const auto rhs_end = rhs.end();
    for( ; lhs_it != lhs_end && rhs_it != rhs_end; ) {
        std::tuple<typename LHS::rtype,typename RHS::rtype> t(*lhs_it, *rhs_it);
        //std::cout << "lhs_it="<<(*lhs_it)<<","<<(*rhs_it)<<std::endl;
        this->yield(t);
        ++lhs_it;
        ++rhs_it;
    }
}};

template<typename T>
class Fib : public Generator<T> { public:
void operator()() {
    T a(0), b(1);
    while(true) {
        T tmp = a+b;
        a = b;
        b = tmp;
        //std::cout << "fib_a="<<a<<std::endl;
        this->yield(a);
    }
}};

template<typename T>
using Enumerate = Zip<Inc<int,0>,T>;

int main() {
    for( auto x : Take<50,Enumerate<Fib<unsigned long long>>>() ) {
        std::cout <<std::get<0>(x)<<","<<std::get<1>(x)<<std::endl;
    }
//     for( auto x : Take<Enumerate<Mod<Fib<unsigned long long>,100>>,10>() ) {
//         std::cout <<std::get<0>(x)<<","<<std::get<1>(x)<<std::endl;
//     }
    // Take(10, Zip(Inc<int>(1), Mod(100,Fib<int>())) )
}
