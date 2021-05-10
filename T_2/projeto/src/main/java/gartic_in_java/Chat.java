package gartic_in_java;

import org.jgroups.*;
import org.jgroups.util.Util;

import java.io.*;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

public class Chat  implements Receiver {
    JChannel channel;
    final List<String> state = new LinkedList<String>();
    private Jartic controller;

    // MESSAGES
    static final String MSG_warningStartRound = " ||| WARNING ||| The round will start in few seconds ... ";
    static final String MSG_gotItRight = "Congratulations, you did get it right !";
    static final String MSG_wordToDraw = "WORD TO DRAW-";
    static final String MSG_roundStarted = "ROUND STARTED-";
    static final String MSG_roundFinished = "ROUND FINISHED - PROCESSING -";
    static final String MSG_newLeader = "NEW_LEADER:";
    static final String MSG_playerAdded = "PLAYER_ADDED:";
    static final String MSG_updateScore = "UPDATE_SCORE:";
    static final String MSG_resetScore = "RESET_SCORE";

    public Chat(Jartic jartic) {
        this.controller = jartic;
        this.channel = jartic.channel;
    }

    public void viewAccepted(View new_view) {
        System.out.println("** view: " + new_view);
        controller.updatePlayersInGame();
    }

    public void receive(Message msg) {
        String line = msg.getObject();
        if(line.contains(MSG_resetScore)){
            controller.onReset();
            return;
        }
        if(line.contains(MSG_updateScore) && !controller.isLeader()){
            String[] data = line.split(":", 2);
            System.out.println("The new score is -> " + data[1]);
            controller.onUpdateScoreByText(data[1]);
            return;
        }
        if (line.contains(MSG_newLeader)) {
            String newLeader = line.split(":")[1];
            System.out.println("The new player to draw is: " + newLeader);
            controller.onNewLeader(newLeader);
            return;
        }
        if (line.contains(MSG_roundStarted)) {
            Date ts = new Date(Long.parseLong(line.split("-")[1]));
            System.out.println("Round started at: " + ts + " - finishing in 90 seconds ...");
            //TODO should call some controller method
            return;
        }
        if (line.contains(MSG_warningStartRound) || line.contains(MSG_gotItRight) || line.contains(MSG_roundFinished)) {
            System.out.println(line);
            return;
        }

        System.out.println(line);
        if(controller.isLeader()){
            controller.onAnswerSubmitted(line);
        }
        return;
    }

    public void getState(OutputStream output) throws Exception {
        synchronized (state) {
            Util.objectToStream(state, new DataOutputStream(output));
        }
    }

    @SuppressWarnings("unchecked")
    public void setState(InputStream input) throws Exception {
        List<String> list = (List<String>) Util.objectFromStream(new DataInputStream(input));
        synchronized (state) {
            state.clear();
            state.addAll(list);
        }

    }

    public void send(String msgText){
        try {
            Message msg = new ObjectMessage(null, msgText);
            if(channel == null){
                channel = controller.channel;
            }
            channel.send(msg);
        }catch(Exception e){
            e.printStackTrace();
            System.out.println("Failed in sending msg->"+msgText);
        }
    }
    public void printHelp(){
        System.out.println(" /score to show actual score");
        System.out.println(" /start to begin a round, only possible if leader");

        //TODO could require every player to enter /reset
        System.out.println(" /reset to set the points of each player to 0");

        System.out.println(" /quit to exit the game");
    }

    public void eventLoop() {
        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
        System.out.flush();
        System.out.println(" Welcome to Jartic, a Java Version of Gartic !");
        System.out.println(" At every moment you can run /help to see the commands");
        printHelp();
        System.out.println(" After you see this message -> \n \"" + MSG_roundStarted+"some fake time");
        System.out.println(" Just type in your answer and press ENTER ");
        System.out.println(" If you get it right, you will receive this message-> \n"+MSG_gotItRight);
        System.out.println(" Otherwise keep trying !");
        System.out.println(" I Know there is a lot of features and UX to add yet, but it is what I could so far by myself :)");
        System.out.println(" I hope you enjoy it !!!");

        while (true) {
            try {
                String line = in.readLine().toLowerCase();
                // TODO define better strategy with enums to this.
                if (line.startsWith("/help")) {
                    printHelp();
                }else if (line.startsWith("/quit")) {
                    this.controller.onQuit();
                    break;
                }else if (line.equals("/score")) {
                    System.out.println(controller.score.toString());
                }else if (line.equals("/start") && controller.canStartRound()) {
                    System.out.flush();
                    controller.startRound();
                }else if (line.equals("/score")) {
                    System.out.println(controller.score.toString());
                }else if (line.equals("/reset")) {
                    send(MSG_resetScore);
                } else {
                    if(channel == null){
                        channel = controller.channel;
                    }
                    line = channel.getAddressAsString() + ":" + line;
                    send(line);
                }
            } catch (Exception e) {
            }
        }
    }
}
