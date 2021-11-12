#ifndef COMMON_H
#define COMMON_H
#include <QTypeInfo>
#include <array>
#include <QString>

namespace glb {
/*! \brief  Возвращает строку хекс представления байта. */
QString hex(uchar bt);

/*! \brief  Возвращает строку хекс представления массива байт. */
QString hex(const void* const ptr, size_t len, char spacer = ' ');

/*! \brief  Возвращает строку хекс представления массива байт. */
QString hex(const QString & str, char spacer = ' ');

/*! \brief  Возвращает строку хекс представления массива байт. */
QString hex(const QByteArray& array, char spacer = ' ');

/*! \brief  Возвращает строку хекс представления массива байт. */
template<typename T, std::size_t S>
QString hex(const std::array<T,S> & arr, char spacer = ' ') {
    return hex(arr.data(), S, spacer);
}

/*! \brief  Возвращает строку бинарного представления байта. */
QString bin(uchar bt);

/*! \brief  Возвращает строку бинарного представления массива байт. */
QString bin(const void * const ptr, size_t len, char spacer = ' ');

/*! \brief  Возвращает строку бинарного представления массива байт.*/
QString bin(const QByteArray& array, char spacer = ' ');

/*! \brief  Возвращает строку бинарного представления массива байт.*/
template<typename T, std::size_t S>
QString bin(const std::array<T,S> & arr, char spacer = ' ') {
    QString result;
    for(auto i : arr) {
        result += bin(i) + spacer;
    }
    return result.trimmed();
}
/*! \brief  Блокирующее поток ожидание события глобального завершения в течение. */
bool waitQuitFor (int ms);

/*! \brief  Возвращает флаг необходимости завершения работы. */
bool isNeedExit();

/*! \brief  Инициирует глобальное завершение работы. */
void globalExit();

}

#endif // COMMON_H
