

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;
void print(const string& name) {
    cout << "c_value detected:" << name << endl;
}

void print(string& name) {
    cout << "lvalue detected:" << name << endl;
}


void print(string&& name) {
    cout << "rvalue detected:" << name << endl;
}

class System {
    
private:
    string *name;
    
public:
    // constuctor
    System (const char chars[]){
        cout << "constuctor from " << chars << endl;
        name = new string(chars);
    }
    
    // copy constructor
    System (const System &system){
        cout << "copy from " << *(system.name) << endl;
        name = new string(system.name->c_str());
    }
    
    // assign operator
    System& operator= (const System& rsystem) {
        cout << "assign from " << *(rsystem.name) << endl;
        System tmp(rsystem.name->c_str());
        string *s = this->name;
        this->name = tmp.name;
        tmp.name = s;
        return *this; // 取的this指向的变量，返回给外部引用
    }
    
    // move assign operator
    System& operator= (System&& rsystem) {
        this->name = rsystem.name; // 使用了rsystem的name，无需深拷贝
        // rsystem 本身很快就销毁，这里因为已经被this->name指向了，
        // 就设为NULL，防止rsystem销毁时被delete
        rsystem.name = NULL;
        return *this; // 取的this指向的变量，返回给外部引用
    }
    //
    // destructor
    ~System() {
        if (name) {
            cout << "destructor of " << *name << endl;
            delete name;
        }
    }
};

void copy() {
    cout << "- - - - - - - - - - copy start - - - - - - - - - " << endl;
    System xos("xos");
    System xos_1 = xos;
    cout << "- - - - - - - - - - copy end - - - - - - - - - " << endl;
}

void assign() {
    cout << "- - - - - - - - - - assign start - - - - - - - - - " << endl;
    System xos("xos");
    System windows("window");
    windows = xos;  // operator=
    cout << "- - - - - - - - - - assign end - - - - - - - - - " << endl;
}

void rvalue() {
    cout << "- - - - - - - - - - rvalue start - - - - - - - - - " << endl;
    System windows("window");
    windows = System("xos");
    cout << "- - - - - - - - - - rvalue end - - - - - - - - - " << endl;
}



int main(int argc, const char * argv[]) {
    copy();
    assign();
    rvalue();
    return 0;
}
