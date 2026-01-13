# Интернет-магазин на C++ & PostgreSQL

## Цель работы
Разработка полнофункциональной системы интернет-магазина с использованием: C++ и PostgreSQL, демонстрирующая владение ООП, работой с бд и использованием умных указателей

## Описание
Система представляет собой консольное приложение для управления интернет-магазином с поддержкой трех ролей: admin, manager & customer. Реализовано: создание заказов, управление товарами, обработка платежей и аудит действий

## Архитектура проекта

### Иерархия классов пользователей
```C++
class User {} 
class Admin : User {}
class Manager : User {}
class Customer : User {}
```
Где классы: admin, manager & customer являются наследниками от класса User.

### Работа с базой данных:
Подключение к PostgreSQL происходит через libpqxx, реализованны: поддержка транзакций, хранимых процедур, функций 
```C++
template<typename ConnectionString = std::string>
```

### Классы для работы с заказами:
class Order
├── std::vector<std::unique_ptr<OrderItem>> (композиция)
└── std::unique_ptr<Payment> (агрегация)

class OrderItem

## Работа с базой данных
### Структура бд:
Таблицы:
1. users - пользователи системы
- user_id, name, email, role, password_hash  ,loyalty_level

2. products - каталог товаров
- product_id, name, price, stock_quantity

3. orders - заказы
- order_id, user_id, status, total_price, order_date

4. order_items - элементы заказов
- order_item_id, order_id, product_id, quantity, price

5. order_status_history - история статусов
- history_id, order_id, old_status, new_status, changed_at, changed_by

6. audit_log - журнал аудита
- log_id, entity_type, entity_id, operation, performed_by, performed_at, detail

### Хранимые процедуры
1. create_order_procedure - создание заказа с транзакцией
- проверка наличия товара
- расчет итоговой суммы
- атомарное выполнение операций
2. update_order_status_procedure - изменение статуса заказа

### Функции 
1. getOrderStatus - возвращает статус заказа
2. getUserOrderCount - количество заказов пользователя
3. getTotalSpentByUser - общая сумма покупок
4. canReturnOrder - проверка возможности возврата
5. getOrderStatusHistory - история изменений статуса

### Триггеры
1. trigger_update_order_date - обновление даты при смене статуса
2. trigger_update_order_totals - пересчет суммы при изменении цены
3. trigger_order_status_history - запись истории статусов
4. trigger_audit_products - автоматическое логирование

### Механизм транзакций
```C++
db->beginTransaction();
try {
    db->commitTransaction();
} catch (...) {
    db->rollbackTransaction();
    logger->logAudit(...);
}
```

## Умные указатели и STL
### Использование умных указателей
1. std::unique_ptr
Владельцем является Order
```C++
std::unique_ptr<Payment> payment;
```
2. std::shared_ptr
Агрегация
```C++
std::vector<std::shared_ptr<Order>> userOrders;
```

### STL-алгоритмы и лямбда-выражения
1. Фильтрация заказов по статусу:
```C++
auto filterByStatus = [](const std::unique_ptr<Order>& order, 
                        const std::string& target_status) {
    return order->getStatus() == target_status;
};

std::vector<std::unique_ptr<Order>> pending_orders;
std::copy_if(all_orders.begin(), all_orders.end(),
             std::back_inserter(pending_orders),
             [](const auto& order) { 
                 return filterByStatus(order, "pending"); 
             });
```

2. Подсчет суммы заказов
```C++
double total_spent = std::accumulate(orders.begin(), orders.end(), 0.0,
    [](double sum, const std::shared_ptr<Order>& order) {
        return sum + order->getTotalPrice();
    });
```

3. Поиск заказов
```C++
auto it = std::find_if(userOrders.begin(), userOrders.end(),
    [order_id](const std::shared_ptr<Order>& order) {
        return order->getOrderId() == order_id;
    });
```

## Логика ролей и прав доступа
### Возможности ролей
Admin:
- Управление товарами (add, update, delete)
- Просмотр всех заказов
- Создание CSV-отчета
- Изменение статусов заказов
- полный доступ ко всем функциям

Manager:
- Утверждение заказов
- Доступ к истории заказов
- Просмотр заказов в обработке
- Обновление информации о товаре

Customer:
- Создание заказов
- Просмотр своих заказов
- Оплата заказов
- Возврат заказов

### Реализация проверки прав доступа
```C++
auto checkPermission = [](const User& user, 
                         const std::string& required_role,
                         int resource_owner_id = -1) -> bool {
    if (user.getRole() == "admin") return true;
    if (user.getRole() == required_role) {
        if (resource_owner_id == -1) return true;
        return user.getUserId() == resource_owner_id;
    }
    return false;
};
```

## Аудит ролей и прав доступа
1. order_status_history:
- Автоматическое заполнение через триггеры
- Сохранение старого и нового статуса
- Дата изменения и пользователь, который выполняет изменение
2. audit_log:
- Логирование всех критических операций
- Связь с пользователем
- Типы операций: insert, update, delete

### Механизм логирования
```C++
class Logger {
    void logAudit(entity_type, entity_id, operation, details);
    void logOrderStatusChange(order_id, old_status, new_status);
}
```

### Примеры записей аудита
```
entity_type: product, entity_id: 5, operation: update
performed_by: 1 (admin), details: Price updated from 5000 to 5500

entity_type: order, entity_id: 102, operation: update  
performed_by: 3 (customer), details: Status changed to completed
```

## Отчет в формате CSV
Пример отчеты приведен в папке reports. Для читаемости и возможности восприятия в более легкой форме

SQL-запрос:
```SQL
SELECT 
    o.order_id, u.name as customer_name, o.status as order_status,
    o.total_price, o.order_date, osh.old_status, osh.new_status,
    osh.changed_at as status_change_date, uc.name as changed_by_user,
    al.operation as audit_operation, al.performed_at as audit_date,
    al.details
FROM orders o
LEFT JOIN users u ON o.user_id = u.user_id
LEFT JOIN order_status_history osh ON o.order_id = osh.order_id
LEFT JOIN users uc ON osh.changed_by = uc.user_id
LEFT JOIN audit_log al ON o.order_id = al.entity_id 
    AND al.entity_type = 'order'
ORDER BY o.order_date DESC, osh.changed_at DESC;
```

## Сборка и запуск проекта
Требования:
- CMake: 3.10+
- PostgreSQL: 16.0+
- Compiler C\CXX: 14.2.0
- libpqxx: 7.7+

### Инструкция по сборке
1. Клонирование репозитория
'''bash
git clone <repository-url>
cd store
'''

2. Сборка проекта
```bash
mkdir build && cd build
cmake ..
make
``` 

### Инструкция по запуску
1. Запуск
```bash
./store
```
2. Выбор роли
```
Store System
1. Login as Administrator
2. Login as Manager  
3. Login as Customer
4. Exit
```

## Пример работы системы
```
1. Login as Administrator
2. Login as Manager
3. Login as Customer
4. Exit
Select role (1-4):
```
Выберем роль администратора
```
  Admin Panel
1. Add new product
2. Update product information
3. Delete product
4. View all orders
5. View order details
6. Change order status
7. View order status history
8. View audit logs
9. Generate report (CSV)
10. Exit
Select option (1-10):
```
Выберем роль мэнеджера
```
  Manager Panel
1. View pending orders for approval
2. Approve order
3. Update product stock
4. View order details
5. Change order status
6. View approved orders history
7. View order status history
8. Show statistics
9. Exit
Select option (1-9):
```
Выберем роль покупателя
```
  Customer Panel
1. Create new order
2. Add product to order
3. Remove product from order
4. View my orders
5. View order status
6. Make payment
7. Return order
8. View order status history
9. View available products
10. Exit
Select option (1-10):
```