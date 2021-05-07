package rpg_chat;
import org.jgroups.*;
import org.jgroups.demos.Chat;
import org.jgroups.demos.Draw;
import org.jgroups.util.Util;

import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.util.LinkedList;
import java.util.Queue;

public class SimpleChat implements Receiver {
    JChannel channel;
    Integer leader = null;
    String wordToDraw = "";
    Queue<Answer> roundHistory = new LinkedList<Answer>();
    String user_name=System.getProperty("user.name", "n/a");
    Integer ID = null;
    final List<String> state=new LinkedList<String>();

    public void viewAccepted(View new_view) {
        System.out.println("** view: " + new_view);
    }

    public void receive(Message msg) {
        String line=msg.getSrc() + ": " + msg.getObject();
        System.out.println(line);
        synchronized(state) {
            state.add(line);
        }
    }

    public void getState(OutputStream output) throws Exception {
        synchronized(state) {
            Util.objectToStream(state, new DataOutputStream(output));
        }
    }

    @SuppressWarnings("unchecked")
    public void setState(InputStream input) throws Exception {
        List<String> list=(List<String>)Util.objectFromStream(new DataInputStream(input));
        synchronized(state) {
            state.clear();
            state.addAll(list);
        }
        System.out.println("received state (" + list.size() + " messages in chat history):");
        for(String str: list) {
            String[] answer =  str.split(":");
            System.out.println(answer[1]);
            if(isLeader() && wordToDraw == answer[1]){
                // TODO find a way to send feedback back to the user that got it right;
                roundHistory.add(new Answer(answer[0], answer[1]));
            }
        }
    }

    /**
     * Allways first joined node is master. This mehod prints message only once.
     */
    private boolean isLeader() {
        if(leader == null){
            leader = 0 ;
        }
        Address leaderAddress = channel.getView().getMembers().get(leader);
        Address currentAddress = channel.getAddress();
        return leaderAddress.equals(currentAddress);
    }

    private void start() throws Exception {
        channel= new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
        channel.setReceiver(this);
        channel.connect("ChatCluster");
        eventLoop();

        JChannel drawChannel= new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
        MyDraw draw = new MyDraw(drawChannel, isLeader());
        draw.setClusterName("DrawCluster");
        draw.go();
        drawChannel.close();
        channel.close();
    }

    private void eventLoop() {
        BufferedReader in=new BufferedReader(new InputStreamReader(System.in));
        while(true) {
            try {
                System.out.print("> "); System.out.flush();
                String line=in.readLine().toLowerCase();
                if(line.startsWith("quit") || line.startsWith("exit")) {
                    break;
                }
                if(line.startsWith("startRound") && wordToDraw.isEmpty() && isLeader() ){
                    // TODO sort word to draw;
                }else {
                    line = channel.getAddressAsUUID() + ":" + line;
                    Message msg = new ObjectMessage(null, line);
                    channel.send(msg);
                }
            }
            catch(Exception e) {
            }
        }
    }


    public static void main(String[] args) throws Exception {
        new SimpleChat().start();
    }
}