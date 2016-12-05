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
        /** Brief \ Permet d'envoyer un message "specimen" pour dépiler la pile
          * Param \
          * Return \ void
          */

        void donneesRecues();
        void connecte();
        void deconnecte();
        void erreurSocket(QAbstractSocket::SocketError erreur);
        void boutonAttaqueClicked(int indexCase);
        void boutonDefenseClicked(int indexCase);

private:
        void decrypte(const QString &messageRecu);
        void envoie(const QString &messageCache);
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
