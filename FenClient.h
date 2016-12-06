#ifndef HEADER_FENCLIENT
#define HEADER_FENCLIENT

#include <QtGui>
#include <QtNetwork>
#include "ui_FenClient.h"
#include <QList>
#include <QPushButton>


class FenClient : public QWidget, private Ui::FenClient
{
    Q_OBJECT

    public:
        FenClient();

    private slots:
        void on_boutonConnexion_clicked();
        void on_boutonEnvoyer_clicked();
        void on_message_returnPressed();
        void on_boutonNettoyer_clicked();
        void on_jeu_clicked();
        void on_pushButton_bug_clicked();

        void donneesRecues();
        void connecte();
        void deconnecte();
        void erreurSocket(QAbstractSocket::SocketError erreur);
        void boutonAttaqueClicked(int indexCase);
        void boutonDefenseClicked(int indexCase);

private:
        /*
        * Décrypte / interprète une donnée reçu
        */
        void decrypte(const QString &messageRecu);

        /*
        * Envoie des données mais, a la différence de on_boutonEnvoyer_clicked(),
        * les données sont envoyés sans l'ajout du pseudo au début de la chaine.
        */
        void envoie(const QString &messageCache);

        /*
        * Converti 53 en F5 par exemple
        */
        QString convertiNUMcaseVersNomCase(int numDeCase);

        QTcpSocket *socket; // Représente le serveur
        quint16 tailleMessage;

        QChar m_aMoiDeJouer;
        ///r :rien (default)
        ///a :je t'attend
        ///n :non terminé
        ///t :l'autre a terminée .le jeu commence

        QList<QPushButton*> boutonAttaque;
        QList<QPushButton*> boutonDefense;
};

#endif
