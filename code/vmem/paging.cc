#include "paging.hh"

// Función a aplicar a cada elemento de la lista de víctimas
void DeleteID(Victim *v, void *arg)
{
    if (v->process == *((SpaceId *)arg))
        v->process = -1;
}

// Borra todas las victimas correspondientes a un process
void DeleteVictims(List<Victim*> *victims, SpaceId process) 
{
    victims->Apply2(DeleteID,(void*) &process);
}

// Devuelve la primera víctima con un spaceID válido
Victim* GiveVictim(List<Victim*> *victims)
{
    Victim *v;
    v = victims->Remove();
    while(v->process == -1)
        v = victims->Remove();
    return v;
}