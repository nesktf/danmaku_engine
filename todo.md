# Game ideas
- fumo racing
- 2hu clone
- sonic clone

## 2hu clone
Que quiero hacer:
- Clon bonito de 2hu que tenga niveles scripteables con lua
- Basicamente un clon de danmakufu pero con lua(?)
- Posiblemente quiera implementar algún lenguaje de scripting custom en el futuro??

### Como hacer y estructurar esta cosa¿?¿?
Primero deberia ver las bases de un bullet hell

Un nivel de bullet hell tipo 2hu es básicamente un campo 2d donde
ocurren eventos en momentos determinados (spawneo de enemigos y danmaku)

A la mitad del nivel existe un evento que pausa temporalmente el contador
de tiempo del nivel y spawnea un midboss

Despues del midboss sigue la segunda fase del nivel, hasta llegar al boss
donde ocurre algo similar al midboss. Al derrotar al boss se muestran los
resultados y se pasa al siguiente nivel

Entonces, la estructura de un nivel de 2hu tiene los siguientes eventos
principales:
1. Fase 1 del nivel (fase normal)
2. Midboss (fase boss)
3. Fase 2 del nivel (fase normal)
4. Boss (fase boss)

A parte de esto, se lleva cuenta del puntaje, graze, power, lives & bombs

#### Fase normal de nivel
Debe ser que llevan algun tipo de contador de tiempo para triggerear eventos.
Algunos de estos eventos pueden incluir:
- Spawnear enemigos
- Spawnear danmaku (lasers & bullets)

Junto a esto los enemigos pueden triggerear algunos eventos extras dependiendo
de su estado actual (efectos de particulas, spawnear danmaku, spawnear mas
enemigos, triggerar efectos de sonido...), como puede ser al morir o al estar
en medio de una secuencia de spawneo de danmaku (podrian por ejemplo modificar
el estado del danmaku en cierto radio alrededor de ellos)

Una vez se termina el contador de tiempo de la fase, se spawnea al boss

#### Fase de boss
Aparece uno o mas bosses, al aparecer puede triggerear una charla con el jugador
o simplemente empezar a atacar. En algunos casos puede modificar la musica del nivel.

Los ataques de un boss se dividen en spells y non-spells. La unica gran diferencia
notable entre ambos es que un spell tiene una animación inicial y da puntos extra
si el jugador no muere y no usa bombs. Los puntos varian dependiendo del tiempo
que se ha tardado el jugador en terminar el ataque.

Cada ataque tiene un tiempo de ejecución que usualmente varía entre 30-120 segundos.
Un ataque es simplemente una secuencia de spawneo de danmaku que se repite dentro
de ese tiempo de ejecucíon. Puede por ejemplo ser simpemente una secuencia que dure
10 segundos y se repite hasta 4 veces en un tiempo de 40 segundos.

Un ataque empieza cuando aparece el boss, o cuando se cumple un threshold de daño
causado por el jugador. Usualmente al boss se le da una cantidad de vida X, y el boss
realiza uno o dos ataques en esa "ronda" de vida. Si a un boss se le da X de vida, el
primer ataque empieza cuando el boss tiene $X_0=x$ de vida (vida completa), y el segundo
ataque comienza cuando el boss tiene $x_1 = a*x$ de vida, donde $a$ es algun porcentaje
de la vida

Un ataque termina cuando el jugador hace que el boss llegue al siguiente threshold
de vida (ya sea que se muera o no) o cuando se acaba el tiempo de ataque. Al terminarse
el último ataque del boss puede continuarse a la fase 2 del nivel (si es midboss) o
terminar el nivel actual (si es boss)

Adicionalmente en algunos juegos de 2hu puede triggerearse un last-spell al final
de la ultima spellcard si el jugador llega a algún puntuaje determinado

### Entidades
Dentro de un nivel existen varias entidades que interactuan con el jugador (que tambien
es una entidad a parte).

Las entidades que se pueden nombrar son
- Jugador
- Danmaku
- Enemigos
- Boss
- Items
- Particulas


#### Danmaku
Un danmaku es simplemente un sprite bonito que el jugador debe esquivar para no morir.
Puede aparecer en forma de bullet o laser.
El danmaku puede ser spawneado por un enemigo, por un boss o por un evento de nivel.

Por lo que he visto suele tener dos tipos de comportamiento, puede spawnear y seguir un
camino pre-establecido o modificarlo en medio de su tiempo de vida dependiendo de
algun evento externo (usualmente el tiempo, pero un enemigo/boss/jugador puede triggerear
esto tambien).
Para simplificar esto tomaré la nomenclatura **stateful** danmaku para referirme al danmaku
que puede cambiar su estado dependiendo de un evento, y **stateless** danmaku para el que
solo sigue las instrucciones que se le dan al tiempo de spawnear.

Independientemente del tipo de danmaku, el mismo tiene un tiempo de vida determinado por
su posición y su estado en relación al entorno. El danmaku inicia su ciclo de vida al
spawnear en alguna coordenada $(x, y)$ dentro del viewport del nivel, y usualmente termina
al morir el jugador o al estar fuera del viewport+algun radio pre-establecido. 
Podrían darse los casos extras en el que el jugador use una spellcard (bomb) y el danmaku
despawnee (pero no debería afectar a todo el danmaku) y un tiempo de vida fijo que dependa de
su estado interno, como en el caso del self-replicating danmaku (vease taisei, logic bomb),
sin embargo este evento es bastante raro.

El danmaku puede spawnear en cualquier punto arbitrario del viewport. Usualmente se eligen
los puntos donde existe un boss o enemigo, pero deberían poder spawnear en cualquier lado
(vease las spellcards de Sakuya en EoSD)

El danmaku sigue un movimiento radial, esto es seguir un camino que inicia en el punto
de spawneo con algún angulo $\theta$ y continuar hasta despawnear o cambiar de estado
en el caso de ser stateful


#### Jugador
El jugador es el sprite de una toja que pude spawnear danmaku para atacar a bosses y enemigos.

Sus interacciones se resumen basicamente en lo siguiente:
- 8 movimientos cardinales. Los movimientos pueden realizarse con dos velocidades fijas
- Spawnea danmaku especial que hace daño a los bosses y enemigos dependiendo del power
- Puede triggerear bombs que afectan el danmaku y bosses del nivel
- Es dañado al tocar danmaku, enemigos y bosses
- Recolecta items para modificar el puntaje y el nivel de power
- Puede tener alguna gimmick en algun caso (vease fairy wars o SA por ej)

El danmaku spawneado por el jugador suele ser stateless y sigue un camino determinado en linea
recta o curvado (teledirigido)

Cuando el jugador se acerca a una distancia X a cierto tipo de danmaku, el contador de graze
aumenta. No debería de ser capaz de grazear multiples veces el mismo danmaku.

#### Enemigos
Un enemigo es un sprite de una cosa fantasiosa (hada/conejo/cualquer cosa graciosa de gensokyo)
que spawnea danmaku capaz de hacer daño al jugador.

Suelen spawnear fuera del viewport y moverse en algún camino pre-establecido. No he visto
muchos enemigos que sean "stateful" como el danmaku, en el sentido que no suelen cambiar
de movimiento a la mitad de su tiempo de vida. Los pocos casos suelen ser en relación
al jugador (vease fairies de Rin en SA) en vez del boss u otros eventos externos.

En cuanto al tiempo de vida creo que son prácticamente iguales al danmaku, despawnean cuando salen
del viewport, cuando muere el jugador, cuando se invoca una spellcard o dependiendo del entorno
del nivel (aunque de este ultimo no recuerdo casos realmente). No creo que sea conveniente
mantener un tiempo de vida en relación a un contador de tiempo (?)

Por lo que he visto la unica forma de atacar que tienen los enemigos es spawnear danmaku desde
su posición actual. Creo que podría implementarse el caso en el que spawnean danmaku de una
posición arbitraria pero medio que quitaría el sentido de los enemigos (?)

Pueden spawnear danmaku stateful o stateless, favoreciendo el stateless


#### Boss
Los bosses son otro sprite de una toja que ataca al jugador y se mueve en algún patrón
dentro del viewport.

Spawnean fuera del viewport y suelen moverse en la zona superior del viewport, aunque hay
excepciones obviamente (fairies of light, yukari, youmu...)

Spawnean danmaku en zonas arbitrarias del viewport en forma cíclica (repetida) dentro de un
rango de tiempo pre-establecido. Es más común spawnear desde la posición actual que tenga
pero de nuevo hay excepciones (spellcards de Marisa en IN por ej)

Cuentan con varias barras de vida que determinan el ataque que utilizan actualmente (vease
estados del nivel mas arriba)

Pueden spawnear danmaku stateful o stateless, favoreciendo el stateful (?)

#### Items
Son sprites de objetos raros que modifican el estado global del nivel. Esto es modificar
el puntaje, el nivel de power, los puntos extra para el last-spell y tal vez alguna
otra gimmick (?)

#### Particulas
Sprites que no interactuan directamente con el jugador ni con el entorno. Simplemente son
para hacer efectos bonitos. No he estudiado su comportamiento en profundidad, pero
asumo que siguen un comportamiento similar al danmaku.

### Bueno pero como se implementa???
Ahora, a partir de lo pre-establecido, voy a establecer un plan inicial para
implementar este jueguito. Siento que dividirlo en fases es lo mejor (?)

#### Fase 1: Render y recursos

#### Fase 2: Lógica del juego

#### Fase 3: Consideraciones extra
