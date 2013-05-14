#include <iostream>
struct IncGen {
    int m_value;
    IncGen() : m_value(0) {}
    const int next() {
        m_value += 1;
        return m_value;
    }   
    bool eof() { return false; }
};
struct FibGen {
    int m_a, m_b; FibGen() : m_a(0), m_b(1) {}
    const int next() {
        int tmp = m_a+m_b;
        m_a=m_b;
        m_b=tmp;
        return m_a;
    }   
    bool eof() { return false; }
};
struct GoldGen {
    FibGen m_g; int m_last; GoldGen() : m_last(m_g.next()) {}
    const double next() {
        const int last = m_last;
        m_last = m_g.next();
        return static_cast<double>(last)/m_last;
    }   
    bool eof() { return false; }
};
template<typename GEN, typename RES>
struct Take {
    int m_left; GEN& m_g; Take(GEN& g, int count) : m_left(count), m_g(g) {}
    RES next() {
        m_left -= 1;
        return m_g.next();
    }   
    bool eof() {
        return m_left<=0;
    }   
};
template<typename A, typename B>
struct Tuple {
    A m_a; B m_b; Tuple(const A& a, const B& b) : m_a(a), m_b(b) {}
    void print(std::ostream& os) const {
        os << "(" << m_a << "," << m_b << ")";
    }   
};
template<typename A, typename B>
std::ostream& operator<<(std::ostream& os, const Tuple<A,B>& t) {
    t.print(os);
}
template<typename GEN, typename RES>
struct Enumerate {
    int m_at; GEN& m_g; Enumerate(GEN& g) : m_at(-1), m_g(g) {}
    Tuple<int,RES> next() {
        m_at += 1;
        return Tuple<int,RES>(m_at, m_g.next());
    }   
    bool eof() {
        return m_g.eof();
    }   
};
int main() {
    GoldGen gg; 
    Take<GoldGen,double> t(gg, 20);
    Enumerate<Take<GoldGen,double>, double> g(t);
    int i = 0;
    double x = 0;
    while(!g.eof()) {
        std::cout << g.next() << std::endl;
    }   
}

