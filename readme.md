Для запуска выполните `make multi` или `make mono`

В первом случае соберется многопоточная версия, во втором оптимизированная под однопоточность.

Бинарники хранятся в папке `bin`.

Параметры запуска

* `-t` - количество потоков (только в `multi` версии, по умолчанию равно максимальному количеству потоков в системе и не может быть больше его)

* `-p` - порт (по умолчанию 2901)

* `-l` - лимит на количество сессий (по умолчанию максимальное беззнаковое 32-битное число)
