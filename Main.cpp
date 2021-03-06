# include "std_lib_facilities.h"

const char number = '8';
const char quit = 'q';
const char print = ';';
const string prompt = "> ";
const string result = "= ";

const char name = 'a';
const char let = 'L';
const string declkey = "let";

class Token
{
public:
    char kind;
    double value;
    string name;

    Token(char ch) : kind { ch } { }
    Token(char ch, double val) : kind { ch }, value { val } { }
    Token(char ch, string n) : kind { ch }, name { n } { }
};

class Token_stream
{
public:
    Token_stream() : full { false }, buffer { ' ' } { }
    Token get();
    void putback(Token t);
    void ignore(char c);
private:
    bool full;
    Token buffer;
};

void Token_stream::putback(Token t)
{
    if (full)
    {
        error("putback() into a full buffer");
    }

    buffer = t;
    full = true;
}

Token Token_stream::get()
{
    if (full)
    {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> ch;

    switch (ch)
    {
        case print:
        case quit:
        case '(':
        case ')':
        case '{':
        case '}':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '!':
        case '=':
            return Token { ch };
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                std::cin.putback(ch);
                double val;
                std::cin >> val;
                return Token { number, val };
            }
        default:
            {
                if (isalpha(ch))
                {
                    string s;
                    s += ch;

                    while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch)))
                    {
                        s += ch;
                    }

                    cin.putback(ch);

                    if (s == declkey)
                    {
                        return Token { let };
                    }

                    return Token { name, s };
                }

                error("Bad token");
                return Token { ' ' };
            }
    }
}

void Token_stream::ignore(char c)
{
    if (full && c == buffer.kind)
    {
        full = false;
        return;
    }

    full = false;

    char ch;
    while (std::cin >> ch)
    {
        if (ch == c)
        {
            return;
        }
    }
}

class Variable
{
public:
    Variable(string var, double val) : name { var } , value { val } { }
    
    string name;
    double value;
};

double expression();
double term();
double factorial();
double primary();
double get_factorial(double val);
void calculate();
void clean_up_mess();
double get_value(string s);
void set_value(string s, double d);
double statement();
double declaration();
double define_name(string var, double val);
bool is_declared(string var);

Token_stream ts;
vector<Variable> var_table;

int main()
{
    try
    {
        define_name("pi", 3.1415926535);
        define_name("e", 2.7182818284);

        calculate();

        keep_window_open();
    }
    catch (exception& e)
    {
        std::cerr << e.what() << std::endl;
        keep_window_open();

        return 1;
    }
    catch (...)
    {
        std::cerr << "exception" << std::endl;
        keep_window_open();

        return 2;
    }
}

double expression()
{
    double left = term();
    Token t = ts.get();

    while (true)
    {
        switch (t.kind)
        {
            case '+':
                {
                    left += term();
                    t = ts.get();
                    break;
                }
            case '-':
                {
                    left -= term();
                    t = ts.get();
                    break;
                }
            default:
                ts.putback(t);
                return left;
        }
    }
}

double term()
{
    double left = factorial();
    Token t = ts.get();

    while (true)
    {
        switch (t.kind)
        {
            case '*':
                {
                    left *= factorial();
                    t = ts.get();
                    break;
                }
            case '/':
                {
                    double d = factorial();

                    if (!d)
                    {
                        error("divide by zero");
                    }

                    left /= d;
                    t = ts.get();
                    break;
                }
            case '%':
                {
                    double d = factorial();

                    if (!d)
                    {
                        error("divide by zero");
                    }

                    left = fmod(left, d);
                    t = ts.get();
                    break;
                }
            default:
                ts.putback(t);
                return left;
        }
    }
}

double factorial()
{
    double left = primary();
    Token t = ts.get();

    while (true)
    {
        switch (t.kind)
        {
            case '!':
                {
                    left = get_factorial(left);
                    t = ts.get();
                    break;
                }
            default:
                ts.putback(t);
                return left;
        }
    }
}

double primary()
{
    Token t = ts.get();

    switch (t.kind)
    {
        case '(':
            {
                double d = expression();
                t = ts.get();

                if (t.kind != ')')
                {
                    error("')' expected");
                }
                return d;
            }
        case '{':
            {
                double d = expression();
                t = ts.get();

                if (t.kind != '}')
                {
                    error("'}' expected");
                }
                return d;
            }
        case number:
            return t.value;
        case '-':
            return -primary();
        case '+':
            return primary();
        case name:
            return get_value(t.name);
        default:
            error("primary expected");
            return 0;
    }
}

double get_factorial(double val)
{
    int result = 1;

    for (int i = 1; i <= val; ++i)
    {
        result *= i;
    }

    return result;
}

void calculate()
{
    while (std::cin)
    try 
    {
        std::cout << prompt;

        Token t = ts.get();

        while (t.kind == print)
        {
            t = ts.get();
        }

        if (t.kind == quit)
        {
            return;
        }
        
        ts.putback(t);
        std::cout << result << statement() << std::endl;
    }
    catch (exception& e)
    {
        std::cerr << e.what() << std::endl;
        clean_up_mess();
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

double get_value(string s)
{
    for (const Variable& v : var_table)
    {
        if (v.name == s)
        {
            return v.value;
        }
    }

    error("get: undefined variable", s);
    return 0;
}

void set_value(string s, double d)
{
    for (Variable& v : var_table)
    {
        if (v.name == s)
        {
            v.value = d;
            return;
        }
    }

    error("set: undefined variable", s);
}

double statement()
{
    Token t = ts.get();

    switch (t.kind)
    {
        case let:
            return declaration();
        default:
            ts.putback(t);
            return expression();
    }
}

double declaration()
{
    Token t = ts.get();

    if (t.kind != name)
    {
        error("name expected in declaration");
    }

    string var_name = t.name;

    Token t2 = ts.get();

    if (t2.kind != '=')
    {
        error("= missing in declaration of ", var_name);
    }

    double d = expression();
    define_name(var_name, d);

    return d;
}

double define_name(string var, double val)
{
    if (is_declared(var))
    {
        error(var, " declared twice");
    }

    var_table.push_back(Variable(var, val));

    return val;
}

bool is_declared(string var)
{
    for (const Variable& v : var_table)
    {
        if (v.name == var)
        {
            return true;
        }
    }

    return false;
}