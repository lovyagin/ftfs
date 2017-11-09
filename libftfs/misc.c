/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include <string.h>
#include <stdlib.h>
#include "ftfs.h"

ft_prms ft_prms_init ()
{
    ft_prms r;
    r.array  = NULL;
    r.length = 0;
    return r;
}

void ft_prms_push (ft_prms *prms, const ft_prm *prm)
{
    ft_prm *i, *e;
    if (ft_prms_is_error(prms)) return;

    for (i = prms->array, e = prms->array + prms->length; i < e; ++i)
        if (prm->cat == i->cat)
        {
            if (prm->cat == FT_CAT_ALL ||
                prm->cat == FT_CAT_OGR ||
                prm->cat == FT_CAT_OTH ||
                (prm->cat == FT_CAT_UID && prm->prc.uid == i->prc.uid) ||
                (prm->cat == FT_CAT_GID && prm->prc.gid == i->prc.gid) ||
                (prm->cat == FT_CAT_PEX && !strcmp(prm->prc.cmd, i->prc.cmd))
               )
            {
                i->allow |= prm->allow;
                i->deny  |= prm->deny;
                return;
            }
        }
    ft_prms_add (prms, prm);
}

void ft_prms_add (ft_prms *prms, const ft_prm *prm)
{
    ft_prm *a, *e;
    if (ft_prms_is_error(prms)) return;

    a = realloc (prms->array, sizeof (ft_prm) * (prms->length + 1));
    if (!a)
    {
        ft_prms_free(prms);
        prms->length = -1uL;
        return;
    }
    prms->array = a;

    e = prms->array + prms->length;
    *e = *prm;
    if (prm->cat == FT_CAT_PEX)
    {
        size_t l = strlen(prm->prc.cmd);
        e->prc.cmd = malloc (l + 1);
        if (!e->prc.cmd)
        {
            ft_prms_free(prms);
            prms->length = -1uL;
            return;
        }
        memcpy(e->prc.cmd, prm->prc.cmd, l+1);
    }
    ++prms->length;
}

void ft_prms_remove (ft_prms *prms, size_t i)
{
    ft_prm *a;
    if (ft_prms_is_error(prms)) return;

    if (i >= prms->length)
    {
        ft_prms_free(prms);
        prms->length = -1uL;
        return;
    }

    if (prms->length == 1)
    {
        ft_prms_free(prms);
        return;
    }

    a = realloc (prms->array, sizeof (ft_prm) * (prms->length - 1));
    if (!a)
    {
        ft_prms_free(prms);
        prms->length = -1uL;
        return;
    }
    prms->array = a;

    prms->array[i] = prms->array[prms->length - 1];
    --prms->length;
}

void ft_prms_join (ft_prms *prms, const ft_prms *source)
{
    ft_prm *p, *q;

    if (ft_prms_is_error(prms)) return;
    if (ft_prms_is_error(source))
    {
        ft_prms_free(prms);
        *prms = *source;
    }

    for (p = source->array, q = source->array + source->length; p<q; ++p)
    {
        ft_prms_push (prms, p);
        if (ft_prms_is_error (prms)) return;
    }
}

void ft_prms_free (ft_prms *prms)
{
    ft_prm *p, *q;
    if (!prms->array) {prms->length = 0; return; }

    for (p = prms->array, q = prms->array + prms->length; p<q; ++p)
    {
        if (p->cat == FT_CAT_PEX) free(p->prc.cmd);
    }

    free (prms->array);
    prms->array  = NULL;
    prms->length = 0;
}

ft_prms ft_prms_error ()
{
    ft_prms r;
    r.array  = NULL;
    r.length = -1uL;
    return r;
}

ft_prm * ft_prms_element (ft_prms *prms, size_t i)
{
    return i < prms->length && !ft_prms_is_error (prms) ? prms->array + i : NULL;
}

const ft_prm * ft_prms_get (const ft_prms *prms, size_t i)
{
    return i < prms->length && !ft_prms_is_error (prms) ? prms->array + i : NULL;
}

size_t ft_prms_size (const ft_prms *prms)
{
    return prms->length == -1uL ? 0 : prms->length;
}

int ft_str_cpy (char *dest, const char *source, size_t size)
{
    size_t len;
    if (!dest) return 0;
    if (!source || (len = strlen(source)) >= size)
    {
        if (size) *dest = '\0';
        return !source ? 1 : 0;
    }
    memcpy (dest, source, len + 1);
    return 1;
}

int ft_str_cat2 (char *dest, const char *s1, const char *s2, size_t size)
{
    size_t l1 = s1 ? strlen(s1) : 0, l2 = s2 ? strlen(s2) : 0;
    if (!dest) return 0;
    if (l1 + l2 >= size)
    {
        if (size) *dest = '\0';
        return 0;
    }
    if (l1) memcpy (dest, s1, l1);
    if (l2) memcpy (dest + l1, s2, l2 + 1); else dest[l1] = '\0';
    return 1;

}

int ft_str_add (char *dest, size_t shift, const char *source, size_t size)
{
    size_t len = source ? strlen(source) : 0;
    if (!dest) return 0;
    if (len + shift >= size)
    {
        if (size) *dest = '\0';
        return 0;
    }
    if (len) memcpy (dest + shift, source, len + 1); else dest[shift] = '\0';
    return 1;
}


int ft_str_cat (char *dest, const char *source, size_t size)
{
    return ft_str_add (dest, strlen(dest), source, size);
}

