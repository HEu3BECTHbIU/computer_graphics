#pragma once

class ICallbacks // интерфейс, инициализирующий функции обратного вызова
{
public:
    //  функция обратного вызова для получения событий от клавиатуры (нажатие спец. клавиш) и мыши
    virtual void SpecialKeyboardCB(int Key, int x, int y) = 0;
    virtual void KeyboardCB(unsigned char Key, int x, int y) = 0; // функция нажатия клавиши
    virtual void PassiveMouseCB(int x, int y) = 0; // функция пассивного действия
    virtual void RenderSceneCB() = 0; // функция отрисовки
    virtual void IdleCB() = 0;
};