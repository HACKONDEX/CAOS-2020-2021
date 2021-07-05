**Problem inf21-2: posix/dl/cpp-class-loader**
===========================================================

Реализуйте механизм динамической загрузки классов C++ по аналогии с Java 
[ https://docs.oracle.com/javase/7/docs/api/java/lang/ClassLoader.html ].

Необходимо реализовать функциональность классов ClassLoader и Class из предлагаемого интерфейса.

Базовый каталог для поиска классов определен в переменной окружения CLASSPATH. Имя класса совпадает с каноническим именем файла библиотеки. Полное имя класса может содержать пространства имет C++, разделяемые символами ::. В этом случае необходимо искать файлы в соответствующих подкаталогах (по аналогии с пакетами в Java или Python).

Пример: класс some::package::ClassInPackage будет находиться в библиотеке $CLASSPATH/some/package/ClassInPackage.so.

Для загруженных классов необходимо уметь создавать их экземляры с помощью метода newInstance. При этом гарантируется, что каждый загружаемый класс имеет конструктор по умолчанию. В загружаемых классах могут быть виртуальные методы и виртуальные деструкторы.

Интерфейс который необходимо реализовать:

    #include <string>
     
    class AbstractClass
    {
        friend class ClassLoader;
    public:
        explicit AbstractClass();
        ~AbstractClass();
    protected:
        void* newInstanceWithSize(size_t sizeofClass);
        struct ClassImpl* pImpl;
    };


    template <class T>
    class Class : public AbstractClass
    {
    public:
        T* newInstance()
        {
            size_t classSize = sizeof(T);
            void* rawPtr = newInstanceWithSize(classSize);
            return reinterpret_cast<T*>(rawPtr);
        }
    };


    enum class ClassLoaderError {
        NoError = 0,
        FileNotFound,
        LibraryLoadError,
        NoClassInLibrary
    };


    class ClassLoader
    {
    public:
        explicit ClassLoader();
        AbstractClass* loadClass(const std::string &fullyQualifiedName);
        ClassLoaderError lastError() const;
        ~ClassLoader();
    private:
        struct ClassLoaderImpl* pImpl;
    };


***
