# Pairings

Delete all pairings

**URL** : `/api/pairings`

**Method** : `DELETE`

**Auth required** : YES


## Success Response

**Code** : `200 OK`

**Content example**

```json
{
  "pairings": "deleted"
}
```

## Error Response

**Condition** : If 'username' and 'password' combination is wrong.

**Code** : `401 Forbidden`