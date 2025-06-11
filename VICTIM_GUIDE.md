# Guide de Démonstration - Victime

Ce guide vous aidera à démontrer le système IoT sécurisé, en simulant le rôle d'une victime potentielle d'attaques.

## Démarrage Rapide

1. Démarrer Mosquitto :
```bash
net start mosquitto
```

2. Lancer le déploiement du scénario souhaité :
```bash
# Pour le scénario non sécurisé (déploie WeatherStation ET SmartLight)
python deploy.py no-security

# Pour le scénario avec mot de passe simple
python deploy.py basic-security

# Pour le scénario sécurisé
python deploy.py secure
```

3. Accéder aux dashboards dans le navigateur :
```
WeatherStation: http://localhost:8080
SmartLight: http://localhost:8081
```

## Scénarios de Démonstration

### Scénario 1 (Non Sécurisé)
- Observer les données sur les deux dashboards
- Noter que les données sont envoyées en clair
- Vérifier que n'importe qui peut se connecter
- Observer les attaques de type :
  - Flood MQTT (ports 1883 et 1884)
  - Fake Publishing
  - Spoofing

### Scénario 2 (Mot de passe simple)
- Observer les tentatives de connexion
- Vérifier les logs du broker :
```bash
type "C:\Program Files\mosquitto\mosquitto.log"
```
- Noter les tentatives de force brute
- Observer les attaques de type :
  - Brute Force
  - MITM
  - Credential Harvesting

### Scénario 3 (Sécurisé)
- Observer la connexion sécurisée
- Vérifier le chiffrement TLS
- Noter la résistance aux attaques
- Observer les alertes de sécurité :
  - Tentatives de connexion non autorisées
  - Tentatives de force brute
  - Tentatives de MITM

## Surveillance des Alertes

1. **Dashboards**
   - WeatherStation (http://localhost:8080)
     - Alertes de sécurité en temps réel
     - Graphiques de tentatives d'attaque
     - Statut des connexions
   - SmartLight (http://localhost:8081)
     - État des lumières
     - Tentatives de contrôle non autorisées
     - Historique des commandes

2. **Station Météo**
   - LED Rouge : Alerte de sécurité
   - LED Jaune : Tentative d'attaque
   - LED Bleue : Connexion sécurisée
   - Buzzer : Alerte sonore pour les attaques critiques

3. **Logs**
   - Logs MQTT : `C:\Program Files\mosquitto\mosquitto.log`
   - Logs Python : `logs/dashboard.log`

## Commandes Utiles

### Vérification du Système
```bash
# Vérifier le statut de Mosquitto
sc query mosquitto

# Voir les connexions actives
netstat -an | findstr "1883"
netstat -an | findstr "1884"

# Voir les processus Python
tasklist | findstr "python"
```

## Dépannage Rapide

1. **Les dashboards ne s'affichent pas**
   - Vérifier que les serveurs Python tournent
   - Vérifier les ports 8080 et 8081
   - Redémarrer avec `python deploy.py <scenario>`

2. **Pas de données MQTT**
   - Vérifier que Mosquitto est en cours d'exécution
   - Redémarrer Mosquitto : `net stop mosquitto && net start mosquitto`
   - Vérifier les topics dans les dashboards

3. **Alertes non fonctionnelles**
   - Vérifier les connexions des LEDs
   - Vérifier le buzzer
   - Redémarrer la station météo

## Notes Importantes

- Chaque scénario déploie automatiquement les deux projets (WeatherStation et SmartLight)
- Les ports sont différents pour chaque projet pour éviter les conflits
- Les logs sont conservés pour analyse
- Les dashboards se mettent à jour automatiquement
- Les alertes sont visuelles et sonores 