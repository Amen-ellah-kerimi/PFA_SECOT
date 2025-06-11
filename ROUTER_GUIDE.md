# Guide de Configuration du Routeur - TP-Link Topnet VDSL

Ce guide vous aidera à configurer votre routeur TP-Link Topnet VDSL pour une démonstration optimale du système IoT sécurisé.

## Configuration de Base

1. **Accès à l'Interface du Routeur**
   - Ouvrez votre navigateur
   - Accédez à `http://192.168.1.1` (adresse par défaut)
   - Identifiants par défaut :
     - Utilisateur : `admin`
     - Mot de passe : `admin`

2. **Configuration du WiFi**
   - Nom du réseau (SSID) : `IoT_Lab_Network`
   - Canal : 6 (pour éviter les interférences)
   - Mode : 802.11n (2.4 GHz)
   - Largeur de bande : 20 MHz
   - Puissance d'émission : Maximum
   - Sécurité : WPA2-PSK
   - Mot de passe : `IoT_Lab_2024`

3. **Configuration IP**
   - Mode DHCP : Activé
   - Plage d'adresses : 192.168.1.100 - 192.168.1.200
   - Masque de sous-réseau : 255.255.255.0
   - Passerelle : 192.168.1.1
   - DNS : 8.8.8.8, 8.8.4.4

## Optimisations pour la Démonstration

1. **Désactivation des Sécurités**
   - Désactiver le pare-feu (déjà fait)
   - Désactiver le filtrage MAC
   - Désactiver le contrôle parental
   - Désactiver la QoS (Qualité de Service)

2. **Configuration des Ports**
   - Ports MQTT : 1883, 1884 (ouverts)
   - Ports Web : 8080, 8081 (ouverts)
   - Ports WebSocket : 9001, 9002 (ouverts)

3. **Isolation du Réseau**
   - Créer un réseau isolé pour la démonstration
   - Désactiver l'isolation des clients AP
   - Activer le mode pont si nécessaire

## Vérification de la Configuration

1. **Test de Connectivité**
```bash
# Vérifier la connexion au routeur
ping 192.168.1.1

# Vérifier la connexion Internet
ping 8.8.8.8

# Vérifier les ports ouverts
netstat -an | findstr "1883"
netstat -an | findstr "1884"
netstat -an | findstr "8080"
netstat -an | findstr "8081"
```

2. **Test du WiFi**
```bash
# Vérifier la force du signal
netsh wlan show interfaces

# Vérifier la connexion
ipconfig /all
```

## Configuration des Périphériques

1. **Station Météo (NodeMCU)**
   - SSID : `IoT_Lab_Network`
   - Mot de passe : `IoT_Lab_2024`
   - IP statique : 192.168.1.100
   - Passerelle : 192.168.1.1
   - DNS : 8.8.8.8

2. **Feux de Circulation (NodeMCU)**
   - SSID : `IoT_Lab_Network`
   - Mot de passe : `IoT_Lab_2024`
   - IP statique : 192.168.1.101
   - Passerelle : 192.168.1.1
   - DNS : 8.8.8.8

3. **Ordinateur de Démonstration**
   - SSID : `IoT_Lab_Network`
   - Mot de passe : `IoT_Lab_2024`
   - IP : DHCP (192.168.1.100-200)

## Dépannage

1. **Problèmes de Connexion**
   - Vérifier que le routeur est en mode 2.4 GHz
   - Vérifier que le canal 6 est libre
   - Redémarrer le routeur si nécessaire
   - Vérifier les logs du routeur

2. **Problèmes de Performance**
   - Réduire la distance entre les appareils
   - Éviter les interférences (micro-ondes, etc.)
   - Vérifier la charge du réseau
   - Monitorer la bande passante

3. **Problèmes de Ports**
   - Vérifier que les ports sont ouverts
   - Vérifier qu'aucun autre service n'utilise les ports
   - Redémarrer les services si nécessaire

## Notes Importantes

- Gardez une copie de la configuration
- Notez les adresses IP statiques
- Documentez les changements de configuration
- Faites des sauvegardes régulières
- Testez la configuration avant la démonstration

## Sécurité Post-Démonstration

1. **Restauration de la Sécurité**
   - Réactiver le pare-feu
   - Réactiver le filtrage MAC
   - Réactiver le contrôle parental
   - Réactiver la QoS

2. **Nettoyage**
   - Supprimer les règles de port forwarding
   - Supprimer les adresses IP statiques
   - Réinitialiser les mots de passe
   - Vider les logs

3. **Vérification**
   - Tester la connectivité Internet
   - Vérifier la sécurité du réseau
   - Confirmer le fonctionnement normal 