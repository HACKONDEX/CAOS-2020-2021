#include "interfaces.h"
#include <algorithm>
#include <dlfcn.h>
#include <malloc.h>
#include <unistd.h>

////// ClassImpl

struct ClassImpl {

    void (*constructor)(void *);
};

////// AbstractClass

AbstractClass::AbstractClass() {
    pImpl = new ClassImpl;
}

AbstractClass::~AbstractClass() {
    delete pImpl;
}

void *AbstractClass::newInstanceWithSize(size_t sizeofClass) {
    void *object_storage = calloc(1, sizeofClass);
    (pImpl->constructor(object_storage));
    return object_storage;
}

////// ClassLoaderImpl

struct ClassLoaderImpl {

    ~ClassLoaderImpl() {
        if (lib_handle_ != nullptr) {
            dlclose(lib_handle_);
        }
    }

    void GetPathAndConstructor(const std::string &class_name);
    void AddNamespace(const std::string &class_name, size_t &start, size_t &end);
    void *GetConstructor();

    std::string lib_path_;
    std::string constructr_name_;

    void *lib_handle_ = nullptr;
    ClassLoaderError error_ = ClassLoaderError::NoError;
    AbstractClass *abstract_class_ptr_;
};

void ClassLoaderImpl::AddNamespace(const std::string &class_name, size_t &start, size_t &end) {
    lib_path_ += '/';
    std::string sub_namespace = class_name.substr(start, end - start);
    lib_path_ += sub_namespace;
    constructr_name_ += std::to_string(sub_namespace.length());
    constructr_name_ += sub_namespace;
    start = end + 2;
}

void ClassLoaderImpl::GetPathAndConstructor(const std::string &class_name) {
    lib_path_ = getenv("CLASSPATH");
    constructr_name_ = "_ZN";
    std::string delimiter = "::";
    size_t start = 0;
    size_t end = 0;
    while ((end = class_name.find(delimiter, start)) < class_name.length()) {
        AddNamespace(class_name, start, end);
    }
    if (start < class_name.length()) {
        end = class_name.length();
        AddNamespace(class_name, start, end);
    }
    lib_path_ += ".so";
    constructr_name_ += "C1Ev";
}

void *ClassLoaderImpl::GetConstructor() {
    if (access(lib_path_.c_str(), F_OK) != 0) {
        error_ = ClassLoaderError::FileNotFound;
        return nullptr;
    }
    lib_handle_ = dlopen(lib_path_.c_str(), RTLD_NOW);
    if (!lib_handle_) {
        error_ = ClassLoaderError::LibraryLoadError;
        return nullptr;
    }
    void *constructor = dlsym(lib_handle_, constructr_name_.c_str());
    if (constructor == nullptr) {
        error_ = ClassLoaderError::NoClassInLibrary;
        dlclose(lib_handle_);
        return nullptr;
    }
    return constructor;
}

////// ClassLoader

ClassLoader::ClassLoader() {
    pImpl = new ClassLoaderImpl;
}

ClassLoader::~ClassLoader() {
    delete pImpl;
}

ClassLoaderError ClassLoader::lastError() const {
    return pImpl->error_;
}

AbstractClass *ClassLoader::loadClass(const std::string &fullyQualifiedName) {
    pImpl->GetPathAndConstructor(fullyQualifiedName);
    void (*constructor)(void *) = reinterpret_cast<void (*)(void *)>(pImpl->GetConstructor());
    AbstractClass *abstract_class = new AbstractClass();
    abstract_class->pImpl->constructor = constructor;
    return abstract_class;
}

